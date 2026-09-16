import unittest

import gate
import gate_summary

POLICY = {"records": 512, "pairs": 31, "minimum_improvement": 0.01, "maximum_regression": 0.03}


def report(verdict="pass", **overrides):
    profiles = {name: {"median_ratio": 1.0, "interval": (0.995, 1.005)} for name in gate.profile_names()}
    profiles["scalar/high/7"] = {"median_ratio": 0.8, "interval": (0.78, 0.82)}
    profiles["scalar/high/5"] = {"median_ratio": 1.1, "interval": (1.05, 1.15)}
    profiles["state/clone/high"] = {"median_ratio": 1.0, "interval": None}
    base = {"verdict": verdict, "eligible_for_merge": verdict == "pass", "policy": POLICY,
            "targets": ["scalar/high/7"], "runs": [{}] * 62, "profiles": profiles,
            "environment": {"machine": "x86_64", "compiler": "g++ 13\nCopyright"}}
    base.update(overrides)
    return base


class SummaryTests(unittest.TestCase):
    def test_highlights_targets_regressions_and_wide_intervals(self):
        text = gate_summary.render(report())
        self.assertIn(gate_summary.MARKER, text)
        self.assertIn("**Verdict: pass.**", text)
        self.assertIn("| `scalar/high/7` | -20.0% | -22.0% to -18.0% | faster (target) |", text)
        self.assertIn("| `scalar/high/5` | +10.0% | +5.0% to +15.0% | **regressed** |", text)
        self.assertIn("| `state/clone/high` | +0.0% | n/a | wide |", text)
        self.assertNotIn("| `batch/high/7/8` | +0.0% | -0.5% to +0.5% | |", text)
        self.assertIn("<details>", text)
        self.assertIn("x86_64, g++ 13", text)

    def test_error_report(self):
        text = gate_summary.render({"verdict": "fail", "error": "command failed (1): cmake"})
        self.assertIn("**Gate error:** command failed (1): cmake", text)
        self.assertNotIn("| Profile", text)

    def test_no_targets(self):
        text = gate_summary.render(report(targets=[]))
        self.assertIn("none declared (any profile counts)", text)


if __name__ == "__main__":
    unittest.main()
