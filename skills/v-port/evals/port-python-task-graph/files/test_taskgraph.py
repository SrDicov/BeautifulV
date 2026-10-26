import unittest

from taskgraph import Kind, Task, TaskError, TaskGraph


class TaskGraphTests(unittest.TestCase):
    def test_order_puts_dependencies_first(self):
        graph = TaskGraph()
        graph.add(Task("deploy", Kind.DEPLOY, deps=["test"]))
        graph.add(Task("test", Kind.TEST, deps=["build"]))
        graph.add(Task("build", Kind.BUILD))
        self.assertEqual(graph.order(), ["build", "test", "deploy"])

    def test_independent_tasks_keep_definition_order(self):
        graph = TaskGraph()
        graph.add(Task("docs", Kind.BUILD))
        graph.add(Task("lint", Kind.BUILD))
        graph.add(Task("unit", Kind.TEST))
        self.assertEqual(graph.order(), ["docs", "lint", "unit"])

    def test_cycle_is_rejected(self):
        graph = TaskGraph()
        graph.add(Task("a", Kind.BUILD))
        graph.add(Task("b", Kind.BUILD, deps=["a"]))
        graph._tasks["a"].deps.append("b")
        with self.assertRaises(TaskError):
            graph.order()

    def test_unknown_dependency_is_rejected(self):
        graph = TaskGraph()
        graph.add(Task("test", Kind.TEST, deps=["missing"]))
        with self.assertRaises(TaskError):
            graph.order()

    def test_duplicate_is_rejected(self):
        graph = TaskGraph()
        graph.add(Task("build", Kind.BUILD))
        with self.assertRaises(TaskError):
            graph.add(Task("build", Kind.TEST))

    def test_summary_groups_by_kind_in_fixed_order(self):
        graph = TaskGraph()
        graph.add(Task("deploy", Kind.DEPLOY, priority=1))
        graph.add(Task("unit", Kind.TEST))
        graph.add(Task("build", Kind.BUILD))
        graph.add(Task("docs", Kind.BUILD))
        self.assertEqual(
            graph.summary(),
            "build: build\n"
            "build: docs\n"
            "test: unit\n"
            "deploy: deploy (priority 1)",
        )

    def test_priority_is_optional(self):
        self.assertIsNone(Task("build", Kind.BUILD).priority)
        self.assertEqual(Task("build", Kind.BUILD, priority=3).priority, 3)


if __name__ == "__main__":
    unittest.main()
