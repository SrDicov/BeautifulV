module main

import os
import strconv
import strings

pub struct Item {
pub mut:
	name  string
	qty   int
	price f64
}

pub struct Inventory {
pub mut:
	items map[string]Item
	dirty bool
}

pub fn load_inventory(path string) Inventory {
	text := os.read_file(path) or { panic(err) }
	mut inv := Inventory{}
	for line in text.split_into_lines() {
		if line == '' {
			continue
		}
		item := parse_line(line)
		inv.items[item.name] = item
	}
	return inv
}

fn parse_line(line string) Item {
	parts := line.split(',')
	qty := strconv.atoi(parts[1]) or { 0 }
	price := strconv.atof64(parts[2]) or { 0.0 }
	return Item{
		name:  parts[0]
		qty:   qty
		price: price
	}
}

pub fn (inv &Inventory) find(name string) Item {
	return inv.items[name]
}

pub fn (mut inv Inventory) add_item(name string, qty int, price f64, force bool) {
	if qty <= 0 && !force {
		return
	}
	inv.items[name] = Item{
		name:  name
		qty:   qty
		price: price
	}
	inv.dirty = true
}

pub fn (mut inv Inventory) mark_clean() {
	inv.dirty = false
}

pub fn (inv &Inventory) save(path string) {
	mut b := strings.new_builder(1024)
	for name, item in inv.items {
		b.write_string('${name},${item.qty},${item.price}\n')
	}
	os.write_file(path, b.str()) or {}
}

pub fn (inv &Inventory) save_async(path string) {
	spawn save_task(inv, path)
}

fn save_task(inv &Inventory, path string) {
	inv.save(path)
}

pub fn (inv &Inventory) report() string {
	mut b := strings.new_builder(1024)
	for name, item in inv.items {
		b.write_string('${name}: ${item.qty} x ${item.price} (${item.kind()}) = ${item.qty * item.price}\n')
	}
	return b.str()
}

pub fn (item &Item) kind() string {
	if item.price > 100.0 {
		return 'expensive'
	}
	return 'cheap'
}

pub fn initials(name string) string {
	return name[0].ascii_str().to_upper()
}

fn main() {
	inv := load_inventory('inventory.csv')
	println(inv.report())
}
