#!/usr/bin/env python3
"""Build known-bad optimization candidates and require rejection by trusted checks."""
import argparse
import json
from pathlib import Path
import subprocess
import sys

import gate


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source",type=Path,default=Path(__file__).resolve().parents[1])
    parser.add_argument("--out",type=Path,required=True)
    args=parser.parse_args()
    source=args.source.resolve(); out=args.out.resolve()
    if out.exists(): parser.error("output must be new")
    out.mkdir(parents=True)
    files=gate.inventory(source)
    root=out/"source"; build=out/"build"
    gate.snapshot(source,files,root)
    env=gate.clean_environment()
    gate.run(["cmake","-S",root,"-B",build,"-G","Ninja","-DCMAKE_BUILD_TYPE=Release","-DPOKER_BUILD_BENCHMARKS=OFF"],out/"configure.log",env=env,timeout=600)
    target=root/"src/poker.cpp"; original=target.read_text()
    mutations=[
        ("constant_rank", "return poker::detail::high(cards, n);", "return 1;", "Rank.GoldenValuesAndKickers"),
        ("ignores_board", "return poker::detail::omaha(h, nh, b, nb);", "return poker::detail::high(h, nh);", "Omaha.*"),
        ("drops_last_batch_row", "i < count; ++i) out[i] = poker_eval_high_unchecked", "i + 1 < count; ++i) out[i] = poker_eval_high_unchecked", "Batch.*"),
        ("wrong_completion", "return finish(value, h, b);", "return 7462;", "Prepared.*")
    ]
    results=[]
    for name,old,new,filter_name in mutations:
        if old not in original: raise RuntimeError(f"mutation anchor missing: {name}")
        target.write_text(original.replace(old,new,1))
        gate.run(["cmake","--build",build,"--target","poker_tests","-j","4"],out/f"{name}-build.log",env=env,timeout=600)
        with (out/f"{name}-test.log").open("wb") as log:
            proc=subprocess.run([str(build/"poker_tests"),f"--gtest_filter={filter_name}"],stdout=log,stderr=subprocess.STDOUT,env=env,timeout=120)
        if proc.returncode==0: raise RuntimeError(f"known-bad candidate escaped: {name}")
        results.append({"mutation":name,"rejected":True,"exit_code":proc.returncode})
    target.write_text(original+'\n#include <time.h>\nextern "C" int clock_gettime(clockid_t, struct timespec* out) noexcept { out->tv_sec=0; out->tv_nsec=0; return 0; }\n')
    gate.run(["cmake","--build",build,"--target","poker_evaluator","-j","4"],out/"clock-build.log",env=env,timeout=600)
    try: gate.audit_symbols(build,out/"clock-symbols.log")
    except gate.GateError: results.append({"mutation":"clock_interposition","rejected":True})
    else: raise RuntimeError("clock override escaped symbol audit")
    # Explicitly exercise native entry points after corrupting the inline kernel;
    # this mutation affects both native and C paths in the baseline.
    target.write_text(original)
    detail=root/"include/poker/detail/portable.hpp"; native=detail.read_text()
    detail.write_text(native.replace("return static_cast<rank>(rank_count -", "return static_cast<rank>(1 + rank_count -",1))
    gate.run(["cmake","--build",build,"--target","poker_tests","-j","4"],out/"native-build.log",env=env,timeout=600)
    with (out/"native-test.log").open("wb") as log:
        proc=subprocess.run([str(build/"poker_tests"),"--gtest_filter=Native.*"],stdout=log,stderr=subprocess.STDOUT,env=env,timeout=120)
    if proc.returncode==0: raise RuntimeError("native rank shift escaped")
    results.append({"mutation":"native_rank_shift","rejected":True})
    (out/"report.json").write_text(json.dumps(results,indent=2)+"\n")
    print(f"Rejected all {len(results)} faulty candidates. {out/'report.json'}")
    return 0

if __name__=="__main__": sys.exit(main())
