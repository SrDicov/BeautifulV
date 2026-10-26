# Review: inventory.v

## Verdict

Not safe to build on yet. The happy path works, but every failure path either panics or
silently invents data, the inventory can be mutated into an inconsistent state from
outside the module, and a background save is detached. Fix the must-fix items before
adding features.

The review ran `v -check inventory.v` (passes), `v fmt -verify inventory.v` (passes), and
`v vet -W inventory.v` (fails: nine public functions are undocumented). there are no tests
to run (`v test` finds none).

## Must fix

### 1. Panics and swallowed errors on every fallible path

`inventory.v:21`, `inventory.v:34-36`, `inventory.v:69`

`load_inventory` panics when the file is missing. `parse_line` indexes `parts[1]` and
`parts[2]` before checking that the line has three fields, and turns a parse failure into
`0`/`0.0`. `save` discards the write error with `or {}`. Bad input therefore either
crashes the process or silently changes the data.

```v
// current
text := os.read_file(path) or { panic(err) }
parts := line.split(',')
qty := strconv.atoi(parts[1]) or { 0 }
os.write_file(path, b.str()) or {}

// fix: propagate, and say which line and field failed
pub fn load_inventory(path string) !Inventory {
	text := os.read_file(path) or { return error('reading ${path}: ${err.msg()}') }
	mut inv := Inventory{}
	for line in text.split_into_lines() {
		item := parse_line(line) or { return error('${path}: ${err.msg()}') }
		inv.items[item.name] = item
	}
	return inv
}

fn parse_line(line string) !Item {
	parts := line.split(',')
	if parts.len != 3 {
		return error('expected 3 fields, got ${parts.len}')
	}
	qty := strconv.atoi(parts[1]) or { return error('bad quantity ${parts[1]}') }
	price := strconv.atof64(parts[2]) or { return error('bad price ${parts[2]}') }
	return Item{ name: parts[0], qty: qty, price: price }
}
```

`save` should return `!` and let the caller print or handle the message.

### 2. The module has no tests

`inventory.v` has no `_test.v` file, so the parser, the lookup, the report, and every
error path are unverified. The code that crashes on malformed input is exactly the code
with no test.

Add `inventory_test.v` with cases for a normal CSV, an empty file, a missing file, a
malformed line, a bad quantity, a bad price, a duplicate key, and a missing lookup. Call
the functions directly; do not test only through `main`.

### 3. `save_async` detaches the thread

`inventory.v:72-74`

`spawn` returns a handle. Throwing it away detaches the thread, so `main` can exit before
the file is written, and a panic in the thread kills the process.

```v
// fix: return the handle so the caller can wait
pub fn (inv &Inventory) save_async(path string) thread {
	return spawn save_task(inv, path)
}

fn save_task(inv &Inventory, path string) {
	inv.save(path) or { eprintln('saving ${path}: ${err.msg()}') }
}

// caller
h := inv.save_async('out.csv')
h.wait()
```

If the write does not need to be asynchronous, the simplest fix is to delete
`save_async` and call `save` directly.

### 4. `find` cannot distinguish missing from present

`inventory.v:44-46`

A missing key returns a zero `Item` (`name: ''`, `qty: 0`, `price: 0`), which is also a
valid item. Return an option:

```v
pub fn (inv &Inventory) find(name string) ?Item {
	if name !in inv.items {
		return none
	}
	return inv.items[name]
}
```

### 5. `initials` panics on an empty name and splits a UTF-8 rune

`inventory.v:95-97`

`name[0]` is the first **byte**. An empty string panics; a name starting with `é` or an
emoji returns half a character.

```v
pub fn initials(name string) string {
	runes := name.runes()
	if runes.len == 0 {
		return ''
	}
	return runes[0].str().to_upper()
}
```

### 6. Public mutable state with no validation

`inventory.v:7-18`

`pub mut:` lets any caller set `qty` to `-5`, `price` to `-1`, or replace the whole map.
The type has invariants (positive quantity, non-negative price) and cannot enforce them.

Keep the fields private, expose read-only accessors, and validate in a constructor or in
`add_item`:

```v
pub struct Item {
pub:
	name  string
	qty   int
	price f64
}
// construct through Item.new(name, qty, price) !Item
```

The map should be `items map[string]Item` (private) with `find`, `len`, and `report`
methods.

### 7. Public API is undocumented, so `v vet -W` fails

`inventory.v:20`, `44`, `48`, `60`, `64`, `72`, `80`, `88`, `95`

Every public function needs a comment immediately above it starting with its name:

```v
// load_inventory reads a CSV of name,qty,price and indexes it by name.
pub fn load_inventory(path string) !Inventory {
```

Use `v doc ./...` or `v missdoc` to see the remaining gaps.

## Should fix

### 8. The `force bool` parameter has no meaning at the call site

`inventory.v:48-51`

`add_item('x', 1, 2.0, true)` does not say what `true` allows. Use an enum:

```v
pub enum AddMode {
	reject
	force
}

pub fn (mut inv Inventory) add_item(name string, qty int, price f64, mode AddMode) ! {
```

### 9. `kind` returns a stringly typed mode

`inventory.v:88-93`

A typo in `'expensive'` compiles. Use an enum:

```v
pub enum ItemKind {
	cheap
	expensive
}

pub fn (item &Item) kind() ItemKind {
	return if item.price > 100.0 { .expensive } else { .cheap }
}
```

### 10. Report order depends on the input file

`inventory.v:80-86`

V maps preserve insertion order, so the report is deterministic for one file but changes
when the same items arrive in a different order. If the report is diffed or snapshotted,
sort by name:

```v
mut names := inv.items.keys()
names.sort()
for name in names {
	item := inv.items[name]
	// ...
}
```

### 11. The 1024-byte builder capacity is a guess

`inventory.v:65`, `81`

Preallocate from a measured estimate, or use the default `strings.new_builder(0)` and let
it grow. A magic number that is too small causes a reallocation; too large wastes memory.

## Consider

- `main` panics through `load_inventory`; after the fix it should print the error and
  exit 1.
- `parse_line` and `save_task` have no tests. The parser is the part most likely to break.
- The CSV format has no escaping; a name containing a comma cannot round-trip. Document
  that limitation or quote fields.
- The `dirty` flag is written but never read. Either use it or remove it.

## What is already good

- The code is split into small functions (`load_inventory`, `parse_line`, `save`,
  `report`) with clear names.
- `strings.Builder` is the right tool for the report and the save buffer.
- Mutating methods such as `add_item` and `mark_clean` take a `mut` receiver, so their
  changes reach the caller; read-only methods take `&Inventory` where a copy would be
  wasteful.
- The map keyed by name gives O(1) lookup and a natural duplicate policy.
- `main` is short; the module has no hidden global state.
