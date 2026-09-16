import unittest

import pr_scope


class ScopeTests(unittest.TestCase):
    def test_implementation_only(self):
        self.assertTrue(pr_scope.implementation_only(["src/poker.cpp", "include/poker/detail/portable.hpp"]))
        self.assertTrue(pr_scope.implementation_only(["src/kernels/avx2.S"]))

    def test_maintenance_and_empty(self):
        self.assertFalse(pr_scope.implementation_only([]))
        self.assertFalse(pr_scope.implementation_only(["src/poker.cpp", "tests/tests.cpp"]))
        self.assertFalse(pr_scope.implementation_only(["README.md"]))
        self.assertFalse(pr_scope.implementation_only(["src/notes.md"]))
        self.assertFalse(pr_scope.implementation_only([".github/workflows/ci.yml"]))

    def test_targets(self):
        self.assertEqual(pr_scope.declared_targets("Benchmark targets: scalar/high/7, batch/high/7/1024"),
                         ["scalar/high/7", "batch/high/7/1024"])
        self.assertEqual(pr_scope.declared_targets("Poker targets: `scalar/high/7`"), ["scalar/high/7"])
        self.assertEqual(pr_scope.declared_targets("no declaration"), [])
        self.assertEqual(pr_scope.declared_targets("Benchmark targets: made/up"), [])
        self.assertEqual(pr_scope.declared_targets("Benchmark targets: scalar/high/7, scalar/high/7"), [])
        self.assertEqual(pr_scope.declared_targets("Benchmark targets: a\nBenchmark targets: b"), [])
        self.assertEqual(pr_scope.declared_targets(None), [])


if __name__ == "__main__":
    unittest.main()
