# Latest main benchmark

Tested source commit: `ec890176445fa2d7277a1940846ca369d52a72a6`

Recorded: 2026-09-16T19:17:28.235060+00:00. Samples: 5; target records per profile: 512.

Machine: x86_64. Compiler: c++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0.

Workflow: https://github.com/svlocks/poker/actions/runs/35139147842

These are medians and observed ranges from a portable Release build. Hosted runners can differ between runs. This snapshot does not certify a speedup or authorize a merge. All timed outputs and required workloads were validated.

Times include harness overhead. An item is a hand evaluation, except state profiles (one state operation) and startup (one initialization). Simulation profiles include dealing and accumulation; multiply time per item by player count to obtain time per trial.

| Profile | Median ns/item | Observed min–max ns/item | Items/s |
| --- | ---: | ---: | ---: |
| batch/high/5/1 | 8.2 | 8.1–13.9 | 121,933,794 |
| batch/high/5/1024 | 2.5 | 2.2–3.3 | 399,375,975 |
| batch/high/5/128 | 3.1 | 3.0–3.5 | 320,601,127 |
| batch/high/5/31 | 3.0 | 3.0–3.5 | 328,553,616 |
| batch/high/5/8 | 3.8 | 3.5–15.0 | 260,559,796 |
| batch/high/6/1 | 18.2 | 15.0–20.8 | 54,953,311 |
| batch/high/6/1024 | 4.7 | 4.0–6.9 | 215,035,699 |
| batch/high/6/128 | 5.8 | 4.9–7.3 | 172,216,616 |
| batch/high/6/31 | 7.7 | 6.9–8.6 | 129,103,381 |
| batch/high/6/8 | 11.1 | 8.4–19.0 | 89,935,008 |
| batch/high/7/1 | 24.5 | 23.3–30.8 | 40,735,142 |
| batch/high/7/1024 | 11.7 | 9.8–19.0 | 85,540,055 |
| batch/high/7/128 | 12.7 | 10.4–13.4 | 78,624,079 |
| batch/high/7/31 | 13.9 | 11.7–16.7 | 71,955,216 |
| batch/high/7/8 | 16.2 | 16.0–19.2 | 61,686,747 |
| batch/high/9/1 | 57.1 | 56.5–62.3 | 17,508,464 |
| batch/high/9/1024 | 39.1 | 37.5–40.2 | 25,591,683 |
| batch/high/9/128 | 41.4 | 40.7–62.2 | 24,163,481 |
| batch/high/9/31 | 42.8 | 39.5–46.4 | 23,354,753 |
| batch/high/9/8 | 44.3 | 43.3–50.5 | 22,593,884 |
| batch/omaha/4/3/1 | 32.4 | 31.4–36.3 | 30,904,811 |
| batch/omaha/4/3/1024 | 24.9 | 23.1–25.7 | 40,104,962 |
| batch/omaha/4/3/64 | 26.2 | 25.3–26.9 | 38,234,635 |
| batch/omaha/4/3/8 | 26.8 | 25.7–27.4 | 37,277,029 |
| batch/omaha/4/4/1 | 49.9 | 48.6–51.2 | 20,025,031 |
| batch/omaha/4/4/1024 | 45.5 | 45.2–53.4 | 21,962,466 |
| batch/omaha/4/4/64 | 46.7 | 45.8–47.8 | 21,393,056 |
| batch/omaha/4/4/8 | 48.5 | 47.5–48.9 | 20,617,726 |
| batch/omaha/4/5/1 | 80.7 | 76.2–145.2 | 12,392,294 |
| batch/omaha/4/5/1024 | 74.3 | 73.1–89.1 | 13,466,419 |
| batch/omaha/4/5/64 | 75.4 | 72.1–81.0 | 13,258,410 |
| batch/omaha/4/5/8 | 78.2 | 77.1–79.5 | 12,784,658 |
| batch/omaha/5/3/1 | 33.3 | 31.4–33.5 | 29,990,628 |
| batch/omaha/5/3/1024 | 30.1 | 29.4–81.2 | 33,268,356 |
| batch/omaha/5/3/64 | 30.7 | 29.4–34.5 | 32,615,620 |
| batch/omaha/5/3/8 | 31.8 | 30.1–35.3 | 31,492,188 |
| batch/omaha/5/4/1 | 65.4 | 62.2–68.9 | 15,295,453 |
| batch/omaha/5/4/1024 | 59.9 | 58.6–64.2 | 16,700,099 |
| batch/omaha/5/4/64 | 58.7 | 57.2–63.0 | 17,041,672 |
| batch/omaha/5/4/8 | 61.8 | 60.9–65.1 | 16,191,259 |
| batch/omaha/5/5/1 | 103.2 | 101.3–107.3 | 9,694,216 |
| batch/omaha/5/5/1024 | 96.1 | 93.8–103.8 | 10,404,601 |
| batch/omaha/5/5/64 | 98.7 | 96.9–101.9 | 10,132,996 |
| batch/omaha/5/5/8 | 102.0 | 96.3–127.8 | 9,804,485 |
| batch/omaha/6/3/1 | 40.8 | 40.4–42.4 | 24,509,335 |
| batch/omaha/6/3/1024 | 37.6 | 37.4–39.4 | 26,560,838 |
| batch/omaha/6/3/64 | 40.1 | 37.2–42.0 | 24,934,255 |
| batch/omaha/6/3/8 | 42.0 | 38.9–45.6 | 23,830,579 |
| batch/omaha/6/4/1 | 80.4 | 79.6–88.6 | 12,432,616 |
| batch/omaha/6/4/1024 | 74.6 | 73.1–77.8 | 13,413,149 |
| batch/omaha/6/4/64 | 75.9 | 74.8–80.6 | 13,169,063 |
| batch/omaha/6/4/8 | 82.5 | 79.2–87.2 | 12,127,816 |
| batch/omaha/6/5/1 | 130.3 | 128.6–148.9 | 7,674,321 |
| batch/omaha/6/5/1024 | 121.6 | 120.0–125.9 | 8,223,248 |
| batch/omaha/6/5/64 | 124.8 | 123.3–126.7 | 8,014,777 |
| batch/omaha/6/5/8 | 128.5 | 121.3–130.4 | 7,780,919 |
| native/high/5 | 24.6 | 23.7–29.9 | 40,618,802 |
| native/high/6 | 46.7 | 43.6–56.3 | 21,415,426 |
| native/high/7 | 80.2 | 72.8–118.6 | 12,466,824 |
| parallel/high/7/1 | 115.0 | 102.6–431.2 | 8,694,323 |
| parallel/high/7/2 | 75.9 | 67.7–122.1 | 13,170,418 |
| parallel/high/7/4 | 109.7 | 103.3–215.3 | 9,113,401 |
| parallel/omaha/4/5/1 | 149.0 | 144.1–159.9 | 6,712,465 |
| parallel/omaha/4/5/2 | 138.6 | 119.5–148.5 | 7,212,791 |
| parallel/omaha/4/5/4 | 122.6 | 111.6–207.3 | 8,156,633 |
| prepared/high/2/1 | 31.5 | 27.4–33.4 | 31,714,569 |
| prepared/high/2/64 | 13.7 | 11.1–14.3 | 73,069,787 |
| prepared/high/2/8 | 14.9 | 14.8–18.4 | 67,059,594 |
| prepared/high/5/1 | 17.1 | 15.2–18.2 | 58,354,228 |
| prepared/high/5/64 | 7.5 | 5.9–8.7 | 133,263,925 |
| prepared/high/5/8 | 10.4 | 9.4–12.1 | 96,240,602 |
| prepared/high/6/1 | 16.4 | 14.7–33.3 | 60,879,905 |
| prepared/high/6/64 | 5.6 | 4.4–7.0 | 178,895,877 |
| prepared/high/6/8 | 9.1 | 6.8–12.3 | 109,753,483 |
| prepared/omaha/4/1 | 104.9 | 101.0–107.6 | 9,536,226 |
| prepared/omaha/4/128 | 57.2 | 56.6–59.9 | 17,468,441 |
| prepared/omaha/4/16 | 65.8 | 64.1–68.8 | 15,208,674 |
| prepared/omaha/5/1 | 110.6 | 109.0–119.9 | 9,044,338 |
| prepared/omaha/5/128 | 74.1 | 73.0–88.0 | 13,499,262 |
| prepared/omaha/5/16 | 84.1 | 80.7–93.5 | 11,886,797 |
| prepared/omaha/6/1 | 138.8 | 137.0–141.8 | 7,205,179 |
| prepared/omaha/6/128 | 94.2 | 92.9–95.4 | 10,618,441 |
| prepared/omaha/6/16 | 109.4 | 99.4–113.9 | 9,141,062 |
| scalar/high/5 | 58.8 | 58.5–61.6 | 16,994,158 |
| scalar/high/6 | 108.3 | 101.3–123.7 | 9,235,876 |
| scalar/high/7 | 146.2 | 137.9–203.7 | 6,841,353 |
| scalar/omaha/4/3 | 110.0 | 98.2–124.6 | 9,092,846 |
| scalar/omaha/4/4 | 64.6 | 60.7–70.9 | 15,483,715 |
| scalar/omaha/4/5 | 85.4 | 83.1–134.2 | 11,704,997 |
| scalar/omaha/5/3 | 46.1 | 45.1–47.4 | 21,714,237 |
| scalar/omaha/5/4 | 76.4 | 73.3–78.9 | 13,091,616 |
| scalar/omaha/5/5 | 111.5 | 109.4–113.6 | 8,970,495 |
| scalar/omaha/6/3 | 58.9 | 55.9–68.1 | 16,978,942 |
| scalar/omaha/6/4 | 95.5 | 93.6–100.3 | 10,467,137 |
| scalar/omaha/6/5 | 141.7 | 135.7–148.9 | 7,058,175 |
| shared/holdem/3/2 | 16.1 | 15.3–16.3 | 62,143,464 |
| shared/holdem/3/6 | 4.4 | 3.5–4.6 | 226,216,572 |
| shared/holdem/3/9 | 3.0 | 2.9–3.2 | 337,056,503 |
| shared/holdem/4/2 | 26.2 | 25.5–26.9 | 38,146,327 |
| shared/holdem/4/6 | 12.9 | 12.7–13.4 | 77,477,478 |
| shared/holdem/4/9 | 8.6 | 8.1–9.5 | 116,089,613 |
| shared/holdem/5/2 | 36.6 | 34.7–40.2 | 27,287,747 |
| shared/holdem/5/6 | 19.2 | 18.7–20.7 | 52,037,112 |
| shared/holdem/5/9 | 16.4 | 15.4–17.3 | 61,027,837 |
| shared/omaha/4/3/2 | 25.9 | 20.7–32.8 | 38,562,928 |
| shared/omaha/4/3/6 | 17.2 | 15.9–19.9 | 58,186,739 |
| shared/omaha/4/4/2 | 41.1 | 40.1–53.1 | 24,343,857 |
| shared/omaha/4/4/6 | 32.6 | 31.0–37.5 | 30,641,330 |
| shared/omaha/4/5/2 | 67.5 | 66.9–68.3 | 14,814,386 |
| shared/omaha/4/5/6 | 57.0 | 55.1–60.9 | 17,530,747 |
| shared/omaha/5/3/2 | 29.0 | 27.1–69.3 | 34,468,830 |
| shared/omaha/5/3/6 | 23.0 | 21.6–38.9 | 43,544,304 |
| shared/omaha/5/4/2 | 55.3 | 51.1–57.6 | 18,089,316 |
| shared/omaha/5/4/6 | 47.4 | 43.6–50.0 | 21,076,709 |
| shared/omaha/5/5/2 | 92.2 | 91.7–96.1 | 10,849,296 |
| shared/omaha/5/5/6 | 82.9 | 81.5–110.8 | 12,062,839 |
| shared/omaha/6/3/2 | 36.4 | 34.3–38.4 | 27,488,457 |
| shared/omaha/6/3/6 | 34.0 | 32.2–35.2 | 29,453,736 |
| shared/omaha/6/4/2 | 76.4 | 74.5–92.2 | 13,082,250 |
| shared/omaha/6/4/6 | 64.6 | 59.3–69.8 | 15,480,619 |
| shared/omaha/6/5/2 | 123.8 | 116.6–128.0 | 8,074,564 |
| shared/omaha/6/5/6 | 106.2 | 102.1–118.5 | 9,413,138 |
| simulation/holdem/2 | 169.2 | 156.0–202.2 | 5,911,353 |
| simulation/holdem/6 | 64.8 | 56.0–72.8 | 15,443,239 |
| simulation/omaha/2 | 188.7 | 178.1–223.1 | 5,298,753 |
| simulation/omaha/6 | 133.7 | 130.6–135.6 | 7,478,019 |
| startup/initialize | 443.0 | 383.0–509.0 | 2,257,336 |
| state/clone/high | 12.0 | 10.7–14.8 | 83,211,442 |
| state/clone/omaha | 12.1 | 11.5–13.2 | 82,567,328 |
| state/extend/high | 18.0 | 17.3–21.5 | 55,615,903 |
| state/extend/omaha | 24.5 | 21.8–25.6 | 40,735,142 |
| total/high/2/1 | 35.4 | 33.5–38.4 | 28,209,366 |
| total/high/2/64 | 11.6 | 10.3–12.8 | 86,559,594 |
| total/high/2/8 | 16.8 | 15.2–17.5 | 59,500,290 |
| total/high/5/1 | 25.7 | 23.1–32.5 | 38,885,092 |
| total/high/5/64 | 8.7 | 7.5–20.2 | 114,438,981 |
| total/high/5/8 | 11.1 | 8.8–13.3 | 90,156,718 |
| total/high/6/1 | 26.2 | 22.0–40.0 | 38,152,012 |
| total/high/6/64 | 5.6 | 4.8–8.4 | 177,408,177 |
| total/high/6/8 | 10.3 | 8.2–14.7 | 97,227,497 |
| total/omaha/4/1 | 100.8 | 98.0–109.9 | 9,924,596 |
| total/omaha/4/128 | 57.3 | 55.2–58.2 | 17,462,483 |
| total/omaha/4/16 | 67.1 | 63.0–88.7 | 14,911,897 |
| total/omaha/5/1 | 124.1 | 121.2–131.1 | 8,059,438 |
| total/omaha/5/128 | 73.8 | 73.0–75.2 | 13,558,963 |
| total/omaha/5/16 | 84.3 | 80.6–87.2 | 11,867,235 |
| total/omaha/6/1 | 146.5 | 143.2–154.8 | 6,827,031 |
| total/omaha/6/128 | 94.3 | 93.7–97.1 | 10,599,756 |
| total/omaha/6/16 | 105.5 | 102.6–110.7 | 9,481,130 |

See `latest.json` for seeds, hashes, resource observations and environment details; `history/` preserves previous snapshots. Raw measurements are retained as workflow artifacts.
