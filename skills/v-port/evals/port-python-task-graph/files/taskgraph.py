from __future__ import annotations

from dataclasses import dataclass, field
from enum import Enum
from typing import Optional


class Kind(Enum):
    BUILD = "build"
    TEST = "test"
    DEPLOY = "deploy"


class TaskError(Exception):
    pass


@dataclass
class Task:
    name: str
    kind: Kind
    deps: list[str] = field(default_factory=list)
    priority: Optional[int] = None


class TaskGraph:
    def __init__(self) -> None:
        self._tasks: dict[str, Task] = {}

    def add(self, task: Task) -> None:
        if task.name in self._tasks:
            raise TaskError(f"duplicate task: {task.name}")
        self._tasks[task.name] = task

    def order(self) -> list[str]:
        result: list[str] = []
        state: dict[str, int] = {}

        def visit(name: str) -> None:
            if name not in self._tasks:
                raise TaskError(f"unknown task: {name}")
            mark = state.get(name, 0)
            if mark == 1:
                raise TaskError(f"dependency cycle at {name}")
            if mark == 2:
                return
            state[name] = 1
            for dep in self._tasks[name].deps:
                visit(dep)
            state[name] = 2
            result.append(name)

        for name in self._tasks:
            visit(name)
        return result

    def summary(self) -> str:
        lines: list[str] = []
        for kind in (Kind.BUILD, Kind.TEST, Kind.DEPLOY):
            for task in self._tasks.values():
                if task.kind is not kind:
                    continue
                suffix = "" if task.priority is None else f" (priority {task.priority})"
                lines.append(f"{kind.value}: {task.name}{suffix}")
        return "\n".join(lines)
