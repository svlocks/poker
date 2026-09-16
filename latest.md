# Latest main benchmark

Tested source commit: `0dac9702296486cd1074cbbf3cdf5733f40fe742`

Recorded: 2026-09-16T19:11:14.322468+00:00. Samples: 5; target records per profile: 512.

Machine: x86_64. Compiler: c++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0.

Workflow: https://github.com/svlocks/poker/actions/runs/35138409195

These are medians and observed ranges from a portable Release build. Hosted runners can differ between runs. This snapshot does not certify a speedup or authorize a merge. All timed outputs and required workloads were validated.

Times include harness overhead. An item is a hand evaluation, except state profiles (one state operation) and startup (one initialization). Simulation profiles include dealing and accumulation; multiply time per item by player count to obtain time per trial.

| Profile | Median ns/item | Observed min–max ns/item | Items/s |
| --- | ---: | ---: | ---: |
| batch/high/5/1 | 27.0 | 24.3–33.9 | 37,085,325 |
| batch/high/5/1024 | 7.0 | 6.1–8.3 | 143,156,716 |
| batch/high/5/128 | 8.2 | 7.7–10.9 | 122,546,675 |
| batch/high/5/31 | 9.7 | 8.2–12.7 | 103,131,116 |
| batch/high/5/8 | 14.5 | 11.1–16.5 | 69,161,151 |
| batch/high/6/1 | 49.3 | 46.8–57.3 | 20,302,958 |
| batch/high/6/1024 | 14.5 | 11.9–17.2 | 69,198,541 |
| batch/high/6/128 | 19.2 | 14.4–24.4 | 51,984,973 |
| batch/high/6/31 | 22.5 | 16.8–27.5 | 44,540,230 |
| batch/high/6/8 | 26.5 | 24.8–33.4 | 37,772,040 |
| batch/high/7/1 | 34.3 | 29.4–66.3 | 29,168,803 |
| batch/high/7/1024 | 18.5 | 17.0–27.3 | 54,079,747 |
| batch/high/7/128 | 21.9 | 19.2–33.5 | 45,628,732 |
| batch/high/7/31 | 24.5 | 21.1–32.1 | 40,871,723 |
| batch/high/7/8 | 24.0 | 20.8–39.8 | 41,751,611 |
| batch/high/9/1 | 65.3 | 63.2–68.8 | 15,319,251 |
| batch/high/9/1024 | 48.8 | 45.9–54.2 | 20,503,374 |
| batch/high/9/128 | 49.0 | 47.1–51.2 | 20,425,260 |
| batch/high/9/31 | 49.0 | 46.6–52.0 | 20,427,940 |
| batch/high/9/8 | 53.1 | 52.0–54.8 | 18,815,920 |
| batch/omaha/4/3/1 | 42.3 | 40.8–53.4 | 23,647,868 |
| batch/omaha/4/3/1024 | 31.1 | 30.2–31.4 | 32,171,919 |
| batch/omaha/4/3/64 | 35.9 | 34.5–57.3 | 27,818,528 |
| batch/omaha/4/3/8 | 37.2 | 35.9–43.2 | 26,868,178 |
| batch/omaha/4/4/1 | 63.6 | 61.0–64.1 | 15,719,505 |
| batch/omaha/4/4/1024 | 52.1 | 50.9–56.2 | 19,208,404 |
| batch/omaha/4/4/64 | 55.8 | 53.8–86.7 | 17,924,660 |
| batch/omaha/4/4/8 | 59.7 | 56.7–75.4 | 16,739,138 |
| batch/omaha/4/5/1 | 97.6 | 96.3–101.4 | 10,243,278 |
| batch/omaha/4/5/1024 | 85.5 | 85.0–94.9 | 11,700,985 |
| batch/omaha/4/5/64 | 90.9 | 89.4–109.3 | 11,004,127 |
| batch/omaha/4/5/8 | 92.5 | 91.5–95.4 | 10,811,039 |
| batch/omaha/5/3/1 | 50.3 | 49.6–51.9 | 19,899,724 |
| batch/omaha/5/3/1024 | 39.0 | 38.4–41.1 | 25,609,604 |
| batch/omaha/5/3/64 | 44.7 | 42.1–45.9 | 22,346,369 |
| batch/omaha/5/3/8 | 46.3 | 45.1–58.9 | 21,617,970 |
| batch/omaha/5/4/1 | 76.1 | 75.6–79.4 | 13,147,421 |
| batch/omaha/5/4/1024 | 66.4 | 65.1–67.9 | 15,068,352 |
| batch/omaha/5/4/64 | 69.7 | 67.9–72.9 | 14,338,925 |
| batch/omaha/5/4/8 | 73.0 | 71.5–75.1 | 13,689,840 |
| batch/omaha/5/5/1 | 125.4 | 124.5–127.6 | 7,972,594 |
| batch/omaha/5/5/1024 | 113.2 | 111.7–133.4 | 8,836,194 |
| batch/omaha/5/5/64 | 115.9 | 114.1–119.9 | 8,628,099 |
| batch/omaha/5/5/8 | 119.8 | 117.4–124.2 | 8,344,878 |
| batch/omaha/6/3/1 | 60.7 | 60.3–84.0 | 16,479,979 |
| batch/omaha/6/3/1024 | 49.4 | 48.5–50.0 | 20,259,175 |
| batch/omaha/6/3/64 | 53.0 | 53.0–54.9 | 18,857,501 |
| batch/omaha/6/3/8 | 55.5 | 54.2–57.4 | 18,013,581 |
| batch/omaha/6/4/1 | 97.3 | 94.3–99.9 | 10,282,363 |
| batch/omaha/6/4/1024 | 85.7 | 83.0–86.2 | 11,663,667 |
| batch/omaha/6/4/64 | 88.6 | 86.7–138.8 | 11,283,747 |
| batch/omaha/6/4/8 | 92.0 | 91.0–94.4 | 10,870,950 |
| batch/omaha/6/5/1 | 156.3 | 155.9–163.2 | 6,398,480 |
| batch/omaha/6/5/1024 | 147.6 | 146.4–154.3 | 6,775,937 |
| batch/omaha/6/5/64 | 147.0 | 146.0–165.5 | 6,803,083 |
| batch/omaha/6/5/8 | 150.9 | 150.4–171.5 | 6,626,545 |
| native/high/5 | 72.7 | 61.2–82.8 | 13,763,441 |
| native/high/6 | 108.3 | 99.8–112.3 | 9,234,543 |
| native/high/7 | 72.0 | 57.9–142.1 | 13,890,773 |
| parallel/high/7/1 | 129.2 | 109.0–154.4 | 7,737,178 |
| parallel/high/7/2 | 69.1 | 53.7–108.4 | 14,481,276 |
| parallel/high/7/4 | 73.6 | 70.2–95.3 | 13,580,542 |
| parallel/omaha/4/5/1 | 215.2 | 162.2–226.2 | 4,646,267 |
| parallel/omaha/4/5/2 | 136.1 | 133.8–137.5 | 7,348,825 |
| parallel/omaha/4/5/4 | 158.8 | 113.0–165.7 | 6,296,811 |
| prepared/high/2/1 | 93.6 | 79.4–105.8 | 10,689,158 |
| prepared/high/2/64 | 37.2 | 31.5–41.2 | 26,896,407 |
| prepared/high/2/8 | 46.5 | 40.4–48.9 | 21,499,055 |
| prepared/high/5/1 | 56.0 | 40.5–69.9 | 17,849,672 |
| prepared/high/5/64 | 19.3 | 17.3–20.9 | 51,832,355 |
| prepared/high/5/8 | 29.2 | 21.6–37.3 | 34,231,464 |
| prepared/high/6/1 | 43.3 | 36.4–46.8 | 23,113,037 |
| prepared/high/6/64 | 10.9 | 10.2–14.3 | 92,086,331 |
| prepared/high/6/8 | 21.5 | 17.3–25.3 | 46,587,807 |
| prepared/omaha/4/1 | 111.1 | 105.0–122.2 | 9,003,623 |
| prepared/omaha/4/128 | 66.7 | 65.2–71.0 | 14,995,314 |
| prepared/omaha/4/16 | 75.0 | 73.4–91.4 | 13,325,699 |
| prepared/omaha/5/1 | 131.4 | 130.8–134.5 | 7,609,310 |
| prepared/omaha/5/128 | 85.6 | 84.3–92.9 | 11,683,629 |
| prepared/omaha/5/16 | 97.3 | 96.1–101.2 | 10,272,255 |
| prepared/omaha/6/1 | 164.3 | 161.8–170.1 | 6,086,688 |
| prepared/omaha/6/128 | 109.5 | 107.7–114.0 | 9,134,049 |
| prepared/omaha/6/16 | 120.0 | 118.1–142.8 | 8,329,944 |
| scalar/high/5 | 140.4 | 130.1–146.0 | 7,121,596 |
| scalar/high/6 | 173.6 | 164.7–185.5 | 5,760,122 |
| scalar/high/7 | 84.0 | 74.4–207.1 | 11,904,208 |
| scalar/omaha/4/3 | 66.8 | 49.0–142.5 | 14,969,009 |
| scalar/omaha/4/4 | 69.9 | 67.9–89.1 | 14,298,880 |
| scalar/omaha/4/5 | 99.7 | 97.5–103.6 | 10,034,297 |
| scalar/omaha/5/3 | 58.2 | 56.1–80.0 | 17,184,091 |
| scalar/omaha/5/4 | 84.8 | 83.7–85.6 | 11,794,246 |
| scalar/omaha/5/5 | 129.3 | 125.6–152.8 | 7,736,009 |
| scalar/omaha/6/3 | 67.6 | 65.5–68.7 | 14,800,254 |
| scalar/omaha/6/4 | 104.4 | 103.7–122.3 | 9,575,463 |
| scalar/omaha/6/5 | 162.0 | 159.1–170.1 | 6,173,435 |
| shared/holdem/3/2 | 48.9 | 42.9–64.0 | 20,433,412 |
| shared/holdem/3/6 | 23.9 | 19.5–26.3 | 41,767,849 |
| shared/holdem/3/9 | 17.6 | 13.0–18.4 | 56,766,626 |
| shared/holdem/4/2 | 66.5 | 64.7–82.7 | 15,048,201 |
| shared/holdem/4/6 | 32.6 | 30.2–34.5 | 30,637,691 |
| shared/holdem/4/9 | 25.4 | 21.8–26.3 | 39,298,299 |
| shared/holdem/5/2 | 69.1 | 64.4–72.8 | 14,468,999 |
| shared/holdem/5/6 | 45.1 | 41.5–61.0 | 22,170,663 |
| shared/holdem/5/9 | 36.6 | 31.3–39.1 | 27,339,586 |
| shared/omaha/4/3/2 | 31.8 | 30.4–32.4 | 31,468,961 |
| shared/omaha/4/3/6 | 27.4 | 24.7–27.9 | 36,528,387 |
| shared/omaha/4/4/2 | 50.7 | 49.7–52.3 | 19,738,618 |
| shared/omaha/4/4/6 | 41.0 | 39.8–42.8 | 24,385,633 |
| shared/omaha/4/5/2 | 79.6 | 78.5–94.4 | 12,562,567 |
| shared/omaha/4/5/6 | 67.4 | 65.5–84.8 | 14,846,785 |
| shared/omaha/5/3/2 | 41.0 | 40.7–42.9 | 24,382,113 |
| shared/omaha/5/3/6 | 37.5 | 36.4–40.0 | 26,644,635 |
| shared/omaha/5/4/2 | 67.9 | 67.7–69.8 | 14,723,221 |
| shared/omaha/5/4/6 | 58.3 | 55.5–61.1 | 17,162,243 |
| shared/omaha/5/5/2 | 107.3 | 105.1–112.6 | 9,323,840 |
| shared/omaha/5/5/6 | 92.3 | 91.1–97.5 | 10,836,011 |
| shared/omaha/6/3/2 | 53.9 | 53.6–64.5 | 18,562,831 |
| shared/omaha/6/3/6 | 51.2 | 46.1–55.2 | 19,538,794 |
| shared/omaha/6/4/2 | 87.9 | 85.8–108.0 | 11,374,239 |
| shared/omaha/6/4/6 | 82.6 | 76.8–83.4 | 12,110,118 |
| shared/omaha/6/5/2 | 139.5 | 135.8–143.2 | 7,170,567 |
| shared/omaha/6/5/6 | 123.6 | 122.3–124.6 | 8,087,774 |
| simulation/holdem/2 | 204.7 | 157.9–221.5 | 4,884,517 |
| simulation/holdem/6 | 90.5 | 77.2–106.6 | 11,048,770 |
| simulation/omaha/2 | 214.1 | 211.0–217.3 | 4,670,042 |
| simulation/omaha/6 | 139.9 | 136.8–142.7 | 7,147,777 |
| startup/initialize | 280.0 | 201.0–311.0 | 3,571,429 |
| state/clone/high | 17.0 | 16.5–17.3 | 58,675,224 |
| state/clone/omaha | 17.2 | 16.3–18.5 | 58,208,277 |
| state/extend/high | 25.7 | 25.5–26.0 | 38,923,521 |
| state/extend/omaha | 27.5 | 27.0–28.6 | 36,348,147 |
| total/high/2/1 | 100.1 | 92.4–130.3 | 9,994,924 |
| total/high/2/64 | 35.4 | 28.5–37.3 | 28,281,043 |
| total/high/2/8 | 46.9 | 37.6–63.3 | 21,337,779 |
| total/high/5/1 | 68.3 | 58.1–69.9 | 14,647,404 |
| total/high/5/64 | 19.6 | 15.2–21.1 | 51,051,949 |
| total/high/5/8 | 28.2 | 23.1–41.2 | 35,464,432 |
| total/high/6/1 | 63.4 | 51.9–82.2 | 15,763,062 |
| total/high/6/64 | 11.8 | 9.8–13.6 | 84,614,113 |
| total/high/6/8 | 21.5 | 17.7–29.3 | 46,498,955 |
| total/omaha/4/1 | 120.4 | 119.2–168.1 | 8,302,927 |
| total/omaha/4/128 | 67.2 | 63.8–74.1 | 14,881,990 |
| total/omaha/4/16 | 77.6 | 74.2–84.9 | 12,885,690 |
| total/omaha/5/1 | 164.7 | 150.4–194.2 | 6,070,163 |
| total/omaha/5/128 | 94.1 | 85.4–106.3 | 10,631,229 |
| total/omaha/5/16 | 101.4 | 96.5–108.6 | 9,863,795 |
| total/omaha/6/1 | 180.0 | 179.7–183.6 | 5,555,375 |
| total/omaha/6/128 | 110.3 | 108.7–116.4 | 9,062,749 |
| total/omaha/6/16 | 123.0 | 121.9–136.1 | 8,131,114 |

See `latest.json` for seeds, hashes, resource observations and environment details; `history/` preserves previous snapshots. Raw measurements are retained as workflow artifacts.
