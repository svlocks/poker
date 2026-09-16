# Latest main benchmark

Tested source commit: `fc8d3ea21a254ec90f30d879888c917a9957478f`

Recorded: 2026-09-16T19:32:29.845565+00:00. Samples: 5; target records per profile: 512.

Machine: x86_64. Compiler: c++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0.

Workflow: https://github.com/svlocks/poker/actions/runs/35140571951

These are medians and observed ranges from a portable Release build. Hosted runners can differ between runs. This snapshot does not certify a speedup or authorize a merge. All timed outputs and required workloads were validated.

Times include harness overhead. An item is a hand evaluation, except state profiles (one state operation) and startup (one initialization). Simulation profiles include dealing and accumulation; multiply time per item by player count to obtain time per trial.

| Profile | Median ns/item | Observed min–max ns/item | Items/s |
| --- | ---: | ---: | ---: |
| batch/high/5/1 | 8.9 | 8.3–10.8 | 112,403,952 |
| batch/high/5/1024 | 2.9 | 2.6–3.1 | 341,789,055 |
| batch/high/5/128 | 3.6 | 3.4–3.9 | 276,457,880 |
| batch/high/5/31 | 3.7 | 3.3–3.7 | 270,672,832 |
| batch/high/5/8 | 4.5 | 4.1–5.1 | 222,608,695 |
| batch/high/6/1 | 18.2 | 16.1–21.1 | 54,935,622 |
| batch/high/6/1024 | 6.2 | 5.2–7.9 | 161,590,658 |
| batch/high/6/128 | 7.9 | 6.9–9.1 | 126,858,277 |
| batch/high/6/31 | 8.2 | 7.9–9.3 | 121,962,509 |
| batch/high/6/8 | 10.6 | 9.9–11.6 | 94,048,494 |
| batch/high/7/1 | 29.5 | 26.6–32.9 | 33,923,011 |
| batch/high/7/1024 | 12.3 | 10.8–15.7 | 81,302,104 |
| batch/high/7/128 | 14.5 | 13.5–17.4 | 68,965,517 |
| batch/high/7/31 | 15.6 | 14.4–24.0 | 64,057,372 |
| batch/high/7/8 | 18.7 | 17.9–21.1 | 53,483,756 |
| batch/high/9/1 | 65.8 | 62.2–67.3 | 15,200,095 |
| batch/high/9/1024 | 43.1 | 41.2–43.8 | 23,175,286 |
| batch/high/9/128 | 45.0 | 44.2–46.7 | 22,210,654 |
| batch/high/9/31 | 48.0 | 46.5–49.6 | 20,851,468 |
| batch/high/9/8 | 50.3 | 47.8–64.1 | 19,875,776 |
| batch/omaha/4/3/1 | 36.8 | 36.4–38.2 | 27,200,765 |
| batch/omaha/4/3/1024 | 27.0 | 26.2–33.4 | 37,069,215 |
| batch/omaha/4/3/64 | 28.8 | 28.3–29.1 | 34,723,635 |
| batch/omaha/4/3/8 | 31.4 | 30.9–32.6 | 31,884,419 |
| batch/omaha/4/4/1 | 57.2 | 57.0–59.8 | 17,480,966 |
| batch/omaha/4/4/1024 | 51.8 | 51.2–56.0 | 19,300,726 |
| batch/omaha/4/4/64 | 54.2 | 53.5–58.1 | 18,450,450 |
| batch/omaha/4/4/8 | 55.6 | 54.7–58.4 | 17,988,265 |
| batch/omaha/4/5/1 | 91.1 | 89.6–93.5 | 10,971,115 |
| batch/omaha/4/5/1024 | 84.4 | 82.9–91.2 | 11,853,361 |
| batch/omaha/4/5/64 | 85.0 | 83.0–95.2 | 11,768,492 |
| batch/omaha/4/5/8 | 87.9 | 84.9–88.9 | 11,374,492 |
| batch/omaha/5/3/1 | 37.5 | 37.0–49.3 | 26,640,304 |
| batch/omaha/5/3/1024 | 33.3 | 32.8–35.5 | 29,996,778 |
| batch/omaha/5/3/64 | 35.9 | 34.8–38.6 | 27,889,748 |
| batch/omaha/5/3/8 | 37.6 | 36.0–37.9 | 26,612,610 |
| batch/omaha/5/4/1 | 73.1 | 70.3–85.6 | 13,678,867 |
| batch/omaha/5/4/1024 | 67.9 | 64.1–68.8 | 14,738,266 |
| batch/omaha/5/4/64 | 69.9 | 68.5–72.1 | 14,305,272 |
| batch/omaha/5/4/8 | 72.2 | 69.2–75.2 | 13,854,313 |
| batch/omaha/5/5/1 | 115.8 | 115.1–130.3 | 8,638,143 |
| batch/omaha/5/5/1024 | 109.1 | 107.8–111.1 | 9,165,279 |
| batch/omaha/5/5/64 | 110.9 | 109.3–122.1 | 9,015,196 |
| batch/omaha/5/5/8 | 114.0 | 113.7–115.6 | 8,769,526 |
| batch/omaha/6/3/1 | 47.6 | 46.2–50.9 | 21,013,749 |
| batch/omaha/6/3/1024 | 43.1 | 40.8–47.0 | 23,209,429 |
| batch/omaha/6/3/64 | 43.6 | 42.1–49.1 | 22,926,742 |
| batch/omaha/6/3/8 | 46.7 | 44.5–48.4 | 21,406,472 |
| batch/omaha/6/4/1 | 93.2 | 87.6–107.7 | 10,725,433 |
| batch/omaha/6/4/1024 | 87.3 | 84.5–102.9 | 11,460,420 |
| batch/omaha/6/4/64 | 89.2 | 87.3–169.3 | 11,205,708 |
| batch/omaha/6/4/8 | 90.3 | 89.9–91.5 | 11,078,654 |
| batch/omaha/6/5/1 | 149.2 | 146.4–159.0 | 6,703,940 |
| batch/omaha/6/5/1024 | 137.3 | 136.8–139.1 | 7,281,571 |
| batch/omaha/6/5/64 | 142.0 | 140.1–144.2 | 7,043,997 |
| batch/omaha/6/5/8 | 145.3 | 141.9–152.8 | 6,880,426 |
| native/high/5 | 28.5 | 28.0–29.2 | 35,073,298 |
| native/high/6 | 54.7 | 50.9–63.7 | 18,266,143 |
| native/high/7 | 99.8 | 88.6–100.1 | 10,019,373 |
| parallel/high/7/1 | 384.9 | 130.6–407.0 | 2,598,101 |
| parallel/high/7/2 | 108.8 | 90.1–123.3 | 9,190,616 |
| parallel/high/7/4 | 103.1 | 75.2–135.8 | 9,698,990 |
| parallel/omaha/4/5/1 | 166.4 | 163.8–212.9 | 6,011,365 |
| parallel/omaha/4/5/2 | 154.7 | 125.0–181.3 | 6,462,199 |
| parallel/omaha/4/5/4 | 147.9 | 128.3–226.5 | 6,759,790 |
| prepared/high/2/1 | 33.3 | 29.4–36.7 | 30,009,964 |
| prepared/high/2/64 | 13.1 | 11.6–15.3 | 76,555,024 |
| prepared/high/2/8 | 17.9 | 14.9–21.3 | 55,718,794 |
| prepared/high/5/1 | 18.3 | 16.1–22.0 | 54,619,159 |
| prepared/high/5/64 | 9.1 | 7.3–10.0 | 110,297,286 |
| prepared/high/5/8 | 10.6 | 10.0–11.5 | 94,762,169 |
| prepared/high/6/1 | 16.7 | 14.6–41.7 | 59,722,384 |
| prepared/high/6/64 | 7.1 | 4.8–8.2 | 140,659,341 |
| prepared/high/6/8 | 10.9 | 7.8–14.3 | 91,412,247 |
| prepared/omaha/4/1 | 119.5 | 111.8–144.2 | 8,370,937 |
| prepared/omaha/4/128 | 63.1 | 61.5–64.3 | 15,849,430 |
| prepared/omaha/4/16 | 75.8 | 71.1–76.9 | 13,187,379 |
| prepared/omaha/5/1 | 127.9 | 124.6–130.3 | 7,818,584 |
| prepared/omaha/5/128 | 83.7 | 80.4–85.2 | 11,941,969 |
| prepared/omaha/5/16 | 95.7 | 94.0–96.6 | 10,452,393 |
| prepared/omaha/6/1 | 156.9 | 155.5–164.4 | 6,374,899 |
| prepared/omaha/6/128 | 106.3 | 104.2–107.1 | 9,407,441 |
| prepared/omaha/6/16 | 118.2 | 113.8–130.6 | 8,456,939 |
| scalar/high/5 | 62.8 | 61.1–72.2 | 15,916,439 |
| scalar/high/6 | 120.3 | 104.1–185.5 | 8,309,260 |
| scalar/high/7 | 171.4 | 146.8–179.0 | 5,835,822 |
| scalar/omaha/4/3 | 107.7 | 104.0–129.2 | 9,281,415 |
| scalar/omaha/4/4 | 71.0 | 70.3–80.5 | 14,093,812 |
| scalar/omaha/4/5 | 96.7 | 96.0–98.3 | 10,346,570 |
| scalar/omaha/5/3 | 52.6 | 51.9–53.8 | 19,006,608 |
| scalar/omaha/5/4 | 87.0 | 83.5–116.1 | 11,500,708 |
| scalar/omaha/5/5 | 128.3 | 124.7–141.5 | 7,792,643 |
| scalar/omaha/6/3 | 64.6 | 64.1–65.9 | 15,469,213 |
| scalar/omaha/6/4 | 109.1 | 107.4–124.5 | 9,168,726 |
| scalar/omaha/6/5 | 162.8 | 156.7–183.4 | 6,144,172 |
| shared/holdem/3/2 | 17.7 | 14.4–18.1 | 56,418,733 |
| shared/holdem/3/6 | 4.9 | 4.5–5.4 | 202,115,159 |
| shared/holdem/3/9 | 3.8 | 3.7–4.0 | 263,347,026 |
| shared/holdem/4/2 | 24.8 | 24.6–25.8 | 40,267,401 |
| shared/holdem/4/6 | 13.0 | 11.5–13.1 | 76,968,974 |
| shared/holdem/4/9 | 9.6 | 8.6–11.2 | 104,353,133 |
| shared/holdem/5/2 | 36.5 | 35.1–38.4 | 27,403,126 |
| shared/holdem/5/6 | 20.8 | 18.4–24.2 | 48,040,220 |
| shared/holdem/5/9 | 16.2 | 14.0–19.2 | 61,829,577 |
| shared/omaha/4/3/2 | 24.5 | 23.8–26.3 | 40,774,070 |
| shared/omaha/4/3/6 | 20.6 | 18.8–22.1 | 48,569,277 |
| shared/omaha/4/4/2 | 45.8 | 43.8–48.5 | 21,841,140 |
| shared/omaha/4/4/6 | 37.5 | 36.6–38.8 | 26,676,317 |
| shared/omaha/4/5/2 | 79.7 | 75.5–81.5 | 12,542,564 |
| shared/omaha/4/5/6 | 66.6 | 62.0–68.6 | 15,010,909 |
| shared/omaha/5/3/2 | 31.4 | 30.7–42.7 | 31,860,610 |
| shared/omaha/5/3/6 | 26.0 | 25.6–31.3 | 38,478,747 |
| shared/omaha/5/4/2 | 62.2 | 60.2–72.3 | 16,083,433 |
| shared/omaha/5/4/6 | 59.3 | 52.8–92.6 | 16,872,118 |
| shared/omaha/5/5/2 | 107.9 | 105.9–109.5 | 9,269,821 |
| shared/omaha/5/5/6 | 91.1 | 88.4–95.1 | 10,981,527 |
| shared/omaha/6/3/2 | 43.2 | 40.7–55.8 | 23,154,848 |
| shared/omaha/6/3/6 | 39.6 | 35.8–41.6 | 25,233,508 |
| shared/omaha/6/4/2 | 84.4 | 79.3–94.4 | 11,854,321 |
| shared/omaha/6/4/6 | 76.1 | 70.4–100.9 | 13,143,148 |
| shared/omaha/6/5/2 | 138.8 | 134.4–145.2 | 7,202,543 |
| shared/omaha/6/5/6 | 119.4 | 115.3–128.4 | 8,376,623 |
| simulation/holdem/2 | 180.9 | 169.5–236.1 | 5,529,426 |
| simulation/holdem/6 | 73.5 | 65.9–78.0 | 13,601,466 |
| simulation/omaha/2 | 233.4 | 204.6–250.7 | 4,285,057 |
| simulation/omaha/6 | 139.5 | 137.9–152.3 | 7,167,639 |
| startup/initialize | 458.0 | 415.0–699.0 | 2,183,406 |
| state/clone/high | 14.5 | 12.4–17.6 | 68,743,287 |
| state/clone/omaha | 13.0 | 12.1–14.1 | 77,131,666 |
| state/extend/high | 19.5 | 18.8–23.7 | 51,374,674 |
| state/extend/omaha | 25.4 | 24.6–26.0 | 39,439,224 |
| total/high/2/1 | 35.3 | 32.5–37.7 | 28,296,673 |
| total/high/2/64 | 12.9 | 11.1–16.1 | 77,224,736 |
| total/high/2/8 | 17.4 | 15.9–26.0 | 57,586,323 |
| total/high/5/1 | 26.5 | 25.4–29.9 | 37,671,989 |
| total/high/5/64 | 7.8 | 6.0–8.8 | 128,934,777 |
| total/high/5/8 | 12.8 | 10.0–14.9 | 77,918,125 |
| total/high/6/1 | 27.1 | 26.4–28.7 | 36,858,398 |
| total/high/6/64 | 6.4 | 4.9–10.1 | 157,393,176 |
| total/high/6/8 | 10.6 | 9.7–14.1 | 93,996,696 |
| total/omaha/4/1 | 115.0 | 106.9–117.6 | 8,693,733 |
| total/omaha/4/128 | 62.0 | 61.1–65.0 | 16,138,692 |
| total/omaha/4/16 | 75.1 | 73.6–85.2 | 13,314,610 |
| total/omaha/5/1 | 143.0 | 140.6–156.3 | 6,993,962 |
| total/omaha/5/128 | 83.2 | 81.1–93.6 | 12,021,037 |
| total/omaha/5/16 | 98.8 | 95.5–113.3 | 10,118,377 |
| total/omaha/6/1 | 170.5 | 168.5–174.8 | 5,865,439 |
| total/omaha/6/128 | 108.7 | 102.9–115.4 | 9,203,833 |
| total/omaha/6/16 | 119.4 | 117.7–122.1 | 8,375,593 |

See `latest.json` for seeds, hashes, resource observations and environment details; `history/` preserves previous snapshots. Raw measurements are retained as workflow artifacts.
