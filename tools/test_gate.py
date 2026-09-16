import copy
import json
from pathlib import Path
import tempfile
import unittest

import gate

POLICY = {"records":512,"pairs":31,"family_error_probability":0.05,"minimum_improvement":0.01,
          "maximum_regression":0.01,"maximum_process_rss_mib":256,"maximum_shared_bytes":1048576,
          "maximum_state_bytes":4096,"maximum_initialization_ns":1000000000}

def valid_report():
    rows=[]
    for name in gate.profile_names():
        if name == "startup/initialize": continue
        count=gate.expected_items(name,512)
        row={"name":name+"/iterations:1/real_time","run_type":"iteration","iterations":1,"verified_items":count,
             "real_time":float(count*100),"time_unit":"ns","items_per_second":1e7}
        if name.startswith("simulation/"): row["verified_trials"]=512
        else: row["verified_batches"]=gate.expected_batches(name,512)
        rows.append(row)
    return {"context":{"poker_protocol":"1","poker_seed":"123","poker_records":"512","poker_smoke":"false",
            "poker_backend":"portable","poker_shared_bytes":"29848","poker_state_bytes":"120","poker_init_ns":"50000"},"benchmarks":rows}

class ReportTests(unittest.TestCase):
    def parse(self, report):
        return gate.parse_measurement(report,seed=123,policy=POLICY,observation={"wall_seconds":1,"peak_rss_bytes":10<<20})
    def test_valid(self):
        self.assertEqual(set(self.parse(valid_report())),set(gate.profile_names()))
    def test_table_free_backend_is_allowed(self):
        report=valid_report(); report["context"]["poker_shared_bytes"]="0"
        self.assertEqual(len(self.parse(report)), len(gate.profile_names()))
    def test_sub_resolution_initialization_is_allowed(self):
        report=valid_report(); report["context"]["poker_init_ns"]="0"
        self.assertEqual(self.parse(report)["startup/initialize"], 1e-9)
    def test_missing_duplicate_extra_and_failed_rows(self):
        for mutation in (lambda r:r["benchmarks"].pop(),lambda r:r["benchmarks"].append(copy.deepcopy(r["benchmarks"][0])),
                         lambda r:r["benchmarks"][0].update(error_occurred=True),lambda r:r["benchmarks"][0].update(run_type="aggregate"),
                         lambda r:r["benchmarks"][0].update(name="unrecognized")):
            report=valid_report(); mutation(report)
            with self.assertRaises(gate.GateError): self.parse(report)
    def test_false_fast_results(self):
        for key,value in (("real_time",0),("real_time",-1),("real_time",float('nan')),("real_time",float('inf')),
                          ("real_time",True),("real_time",1),("iterations",0),("verified_items",0),("verified_items",1),
                          ("items_per_second",float('inf')),("time_unit","minutes")):
            report=valid_report(); report["benchmarks"][0][key]=value
            with self.assertRaises(gate.GateError): self.parse(report)
    def test_provenance_and_resource_tampering(self):
        for key,value in (("poker_seed","124"),("poker_smoke","true"),("poker_records","8"),("poker_backend","pretend"),
                          ("poker_shared_bytes","999999999"),("poker_state_bytes","0"),("poker_init_ns","-1")):
            report=valid_report(); report["context"][key]=value
            with self.assertRaises(gate.GateError): self.parse(report)
    def test_missing_trials_and_batches(self):
        for prefix,key in (("simulation/","verified_trials"),("batch/","verified_batches"),("parallel/","verified_batches"),("state/","verified_batches")):
            report=valid_report(); row=next(r for r in report["benchmarks"] if r["name"].startswith(prefix)); del row[key]
            with self.assertRaises(gate.GateError): self.parse(report)
    def test_malformed_structure(self):
        for report in ([], {}, {"context":[],"benchmarks":[]}, {"context":{},"benchmarks":{}},
                       {**valid_report(), "benchmarks":["invalid"]}):
            with self.assertRaises(gate.GateError): self.parse(report)
    def test_bad_json(self):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/"report.json"
            for data in ('{"x":1,"x":2}','{"x":NaN}','{"x":Infinity}','broken'):
                path.write_text(data)
                with self.assertRaises(gate.GateError): gate.load_json(path)

class DecisionTests(unittest.TestCase):
    def pairs(self, ratio, n=31):
        base={name:1.0 for name in gate.profile_names()}; candidate={name:ratio for name in base}
        return [(base.copy(),candidate.copy()) for _ in range(n)]
    def test_unchanged_is_not_a_speedup(self):
        self.assertEqual(gate.decide(self.pairs(1),["scalar/high/7"],POLICY)["verdict"],"inconclusive")
    def test_improvement(self):
        self.assertEqual(gate.decide(self.pairs(.95),["scalar/high/7"],POLICY)["verdict"],"pass")
    def test_regression_cannot_hide_in_average(self):
        pairs=self.pairs(.5)
        for _,candidate in pairs: candidate["scalar/high/5"]=1.1
        self.assertEqual(gate.decide(pairs,["scalar/high/7"],POLICY)["verdict"],"fail")
    def test_short_noisy_and_diagnostic_never_pass(self):
        self.assertEqual(gate.decide(self.pairs(.9,3),["scalar/high/7"],POLICY)["verdict"],"inconclusive")
        self.assertFalse(gate.decide(self.pairs(.9),["scalar/high/7"],POLICY,True)["eligible_for_merge"])
        pairs=self.pairs(.9)
        for i,(_,candidate) in enumerate(pairs): candidate["scalar/high/7"]=1.2 if i%2 else .8
        self.assertEqual(gate.decide(pairs,["scalar/high/7"],POLICY)["verdict"],"inconclusive")
    def test_unknown_target_and_missing_cases(self):
        for pairs,targets in (([],["scalar/high/7"]),(self.pairs(.9),["invented"]),([({}, {})],["scalar/high/7"])):
            with self.assertRaises(gate.GateError): gate.decide(pairs,targets,POLICY)
    def test_without_targets_any_profile_may_supply_the_gain(self):
        pairs=self.pairs(1)
        for _,candidate in pairs: candidate["batch/high/7/1024"]=.9
        self.assertEqual(gate.decide(pairs,[],POLICY)["verdict"],"pass")
        self.assertEqual(gate.decide(pairs,["scalar/high/7"],POLICY)["verdict"],"inconclusive")
    def test_regression_guard_uses_policy_threshold(self):
        pairs=self.pairs(.9)
        for _,candidate in pairs: candidate["scalar/high/5"]=1.02
        self.assertEqual(gate.decide(pairs,["scalar/high/7"],POLICY)["verdict"],"fail")
        relaxed={**POLICY,"maximum_regression":0.03}
        self.assertEqual(gate.decide(pairs,["scalar/high/7"],relaxed)["verdict"],"pass")
    def test_median_interval(self):
        self.assertIsNone(gate.median_interval([1,2,3],.05))
        self.assertEqual(gate.median_interval(list(range(1,10)),.05),(2,8))

class TrustTests(unittest.TestCase):
    def test_incomplete_exhaustive_run_cannot_succeed_by_exiting_zero(self):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/"census.json"
            valid={"cards":5,"shard":0,"shards":1,"begin":0,"end":2598960,"checked":2598960,"total":2598960}
            path.write_text(json.dumps(valid))
            self.assertEqual(gate.verify_census(path,5,0,1),valid)
            for data in ("", "{}", json.dumps({**valid,"checked":0}), json.dumps({**valid,"shard":False})):
                path.write_text(data)
                with self.assertRaises(gate.GateError): gate.verify_census(path,5,0,1)
    def test_local_environment_is_not_read_or_snapshotted(self):
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory); (root/"CMakeLists.txt").write_text("test")
            # Dangling links prove inventory skips these paths before inspecting
            # or reading them. An ordinary source symlink remains an error.
            (root/".envrc").symlink_to(root/"missing-env")
            (root/".direnv").mkdir()
            (root/".direnv"/"env").symlink_to(root/"missing-cache")
            (root/".local").mkdir()
            (root/".local"/"notes.md").symlink_to(root/"missing-private-note")
            files=gate.inventory(root)
            self.assertEqual(set(files), {"CMakeLists.txt"})
            gate.snapshot(root,files,root/"snapshot")
            self.assertEqual({p.name for p in (root/"snapshot").iterdir()}, {"CMakeLists.txt"})
    def test_protected_policy_oracle_and_workflow_changes(self):
        for path in ("CMakeLists.txt","tests/oracle.cpp","benchmarks/policy.json","tools/gate.py",".github/workflows/ci.yml","include/poker/poker.h"):
            with self.assertRaises(gate.GateError): gate.check_changes({path:"old"},{path:"new"})
    def test_implementation_changes_allowed(self):
        self.assertEqual(gate.check_changes({"src/poker.cpp":"old"},{"src/poker.cpp":"new"}),["src/poker.cpp"])
    def test_disguised_scripts_and_symlinks_rejected(self):
        with self.assertRaises(gate.GateError): gate.check_changes({}, {"src/inject.cmake":"new"})
        with tempfile.TemporaryDirectory() as directory:
            root=Path(directory); (root/"CMakeLists.txt").write_text("test"); (root/"src").mkdir()
            (root/"src/evil.cpp").symlink_to(root/"CMakeLists.txt")
            with self.assertRaises(gate.GateError): gate.inventory(root)

if __name__=="__main__": unittest.main()
