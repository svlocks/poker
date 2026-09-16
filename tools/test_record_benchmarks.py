import copy
from pathlib import Path
import subprocess
import tempfile
import unittest

import gate
import publish_benchmarks
import record_benchmarks


def fixture(commit="a" * 40):
    values = [{name: factor * 1e-6 for name in gate.profile_names()} for factor in (1, 2, 3)]
    return {"schema": 1, "kind": "baseline_snapshot", "eligible_for_merge": False,
            "source_commit": commit, "samples": 3, "records": 512, "runs": [{}, {}, {}],
            "profiles": record_benchmarks.summarize(values), "created_at": "2026-09-15T00:00:00+00:00",
            "workflow_url": "", "environment": {"machine": "test-machine", "compiler": "test-compiler"}}


class SummaryTests(unittest.TestCase):
    def test_medians_and_ranges_use_every_sample(self):
        row = fixture()["profiles"]["scalar/high/7"]
        self.assertEqual(row, {"median_seconds_per_item": 2e-6, "min_seconds_per_item": 1e-6,
                               "max_seconds_per_item": 3e-6})
        self.assertIn("scalar/high/7", record_benchmarks.markdown(fixture()))
        self.assertIn("does not certify a speedup", record_benchmarks.markdown(fixture()))

    def test_incomplete_or_false_measurements_fail(self):
        for samples in ([], [{}], [{name: float("nan") for name in gate.profile_names()}],
                        [{name: 0 for name in gate.profile_names()}]):
            with self.assertRaises(gate.GateError):
                record_benchmarks.summarize(samples)

    def test_wrong_commit_missing_profiles_and_false_approval_fail(self):
        mutations = (lambda r: r.update(source_commit="b" * 40),
                     lambda r: r.update(eligible_for_merge=True),
                     lambda r: r.update(samples=0), lambda r: r["runs"].pop(),
                     lambda r: r["profiles"].pop("scalar/high/7"),
                     lambda r: r["profiles"]["scalar/high/7"].update(min_seconds_per_item=1),
                     lambda r: r["profiles"]["scalar/high/7"].update(median_seconds_per_item=float("inf")))
        for mutation in mutations:
            report = fixture(); mutation(report)
            with self.assertRaises(gate.GateError):
                record_benchmarks.validate_snapshot(report, "a" * 40)


class PublicationTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.origin = self.root / "origin.git"
        self.source = self.root / "source"
        self.command("git", "init", "--bare", str(self.origin))
        self.command("git", "init", "-b", "main", str(self.source))
        self.git("config", "user.name", "test")
        self.git("config", "user.email", "test@example.invalid")
        self.git("remote", "add", "origin", str(self.origin))
        (self.source / "source.txt").write_text("original source")
        self.git("add", "source.txt"); self.git("commit", "-m", "source")
        self.git("push", "origin", "main")
        self.sha = self.git("rev-parse", "HEAD")

    def command(self, *args):
        return subprocess.run(args, check=True, capture_output=True, text=True).stdout.strip()

    def git(self, *args):
        return self.command("git", "-C", str(self.source), *args)

    def test_publish_preserves_source_and_history_and_is_idempotent(self):
        report = fixture(self.sha)
        self.assertTrue(publish_benchmarks.publish(self.source, report, self.sha).startswith("published:"))
        self.assertEqual(self.git("rev-parse", "HEAD"), self.sha)
        self.assertEqual(self.git("status", "--porcelain"), "")
        self.assertEqual(publish_benchmarks.remote_ref(self.source, "refs/heads/main"), self.sha)
        self.git("fetch", "origin", "benchmark-results")
        names = self.git("ls-tree", "-r", "--name-only", "FETCH_HEAD").splitlines()
        self.assertEqual(len(names), 3)
        self.assertIn("latest.md", names); self.assertIn("latest.json", names)
        self.assertNotIn("source.txt", names)
        first = publish_benchmarks.remote_ref(self.source, publish_benchmarks.RESULTS_REF)
        self.assertTrue(publish_benchmarks.publish(self.source, report, self.sha).startswith("unchanged:"))
        self.assertEqual(first, publish_benchmarks.remote_ref(self.source, publish_benchmarks.RESULTS_REF))
        report = copy.deepcopy(report); report["created_at"] = "2026-09-16T00:00:00+00:00"
        self.assertTrue(publish_benchmarks.publish(self.source, report, self.sha).startswith("published:"))
        self.git("fetch", "origin", "benchmark-results")
        self.assertEqual(len(self.git("ls-tree", "-r", "--name-only", "FETCH_HEAD").splitlines()), 4)

    def test_stale_source_does_not_publish(self):
        (self.source / "source.txt").write_text("new source")
        self.git("commit", "-am", "advance main"); self.git("push", "origin", "main")
        self.assertTrue(publish_benchmarks.publish(self.source, fixture(self.sha), self.sha).startswith("stale:"))
        self.assertIsNone(publish_benchmarks.remote_ref(self.source, publish_benchmarks.RESULTS_REF))

    def test_wrong_report_commit_does_not_create_results_branch(self):
        with self.assertRaises(gate.GateError):
            publish_benchmarks.publish(self.source, fixture(), self.sha)
        self.assertIsNone(publish_benchmarks.remote_ref(self.source, publish_benchmarks.RESULTS_REF))


if __name__ == "__main__":
    unittest.main()
