# Latest main benchmark

Tested source commit: `ee99e3803394aa7d936acff1a19c6d63736ef75d`

Recorded: 2026-09-17T15:17:27.348959+00:00. Samples: 5; target records per profile: 512.

Machine: x86_64. Compiler: c++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0.

Workflow: https://github.com/svlocks/poker/actions/runs/35238544888

These are medians and observed ranges from a portable Release build. Hosted runners can differ between runs. This snapshot does not certify a speedup or authorize a merge. All timed outputs and required workloads were validated.

Times include harness overhead. An item is a hand evaluation, except state profiles (one state operation) and startup (one initialization). Simulation profiles include dealing and accumulation; multiply time per item by player count to obtain time per trial.

| Profile | Median ns/item | Observed min–max ns/item | Items/s |
| --- | ---: | ---: | ---: |
| batch/high/5/1 | 13.6 | 12.8–18.9 | 73,320,922 |
| batch/high/5/1024 | 4.3 | 3.9–7.4 | 232,304,900 |
| batch/high/5/128 | 6.0 | 5.4–6.4 | 167,539,267 |
| batch/high/5/31 | 6.3 | 6.0–6.4 | 158,926,417 |
| batch/high/5/8 | 7.2 | 6.6–8.0 | 139,243,949 |
| batch/high/6/1 | 29.9 | 27.7–49.4 | 33,422,547 |
| batch/high/6/1024 | 8.5 | 7.4–10.7 | 118,162,936 |
| batch/high/6/128 | 12.5 | 9.5–16.3 | 79,975,008 |
| batch/high/6/31 | 13.1 | 10.5–18.9 | 76,123,068 |
| batch/high/6/8 | 16.6 | 13.5–25.9 | 60,405,852 |
| batch/high/7/1 | 36.3 | 31.8–70.2 | 27,549,099 |
| batch/high/7/1024 | 15.9 | 12.3–23.9 | 62,976,630 |
| batch/high/7/128 | 21.3 | 17.2–28.2 | 46,843,550 |
| batch/high/7/31 | 19.9 | 17.2–36.6 | 50,195,257 |
| batch/high/7/8 | 21.4 | 18.4–40.2 | 46,753,721 |
| batch/high/9/1 | 60.2 | 52.9–67.2 | 16,602,893 |
| batch/high/9/1024 | 43.3 | 39.7–52.6 | 23,071,897 |
| batch/high/9/128 | 44.5 | 41.7–62.8 | 22,463,037 |
| batch/high/9/31 | 47.2 | 40.7–63.9 | 21,192,745 |
| batch/high/9/8 | 47.1 | 44.0–60.8 | 21,249,222 |
| batch/omaha/4/3/1 | 45.8 | 37.0–54.9 | 21,857,923 |
| batch/omaha/4/3/1024 | 21.4 | 20.4–21.7 | 46,713,197 |
| batch/omaha/4/3/64 | 25.8 | 24.1–29.0 | 38,773,192 |
| batch/omaha/4/3/8 | 29.7 | 28.8–35.5 | 33,620,067 |
| batch/omaha/4/4/1 | 47.3 | 46.2–56.9 | 21,151,781 |
| batch/omaha/4/4/1024 | 36.8 | 36.3–39.6 | 27,189,931 |
| batch/omaha/4/4/64 | 40.3 | 39.1–44.2 | 24,796,590 |
| batch/omaha/4/4/8 | 43.8 | 42.6–47.4 | 22,844,905 |
| batch/omaha/4/5/1 | 73.1 | 68.5–78.0 | 13,686,180 |
| batch/omaha/4/5/1024 | 67.7 | 59.6–81.5 | 14,780,814 |
| batch/omaha/4/5/64 | 63.6 | 62.6–89.2 | 15,729,647 |
| batch/omaha/4/5/8 | 66.6 | 65.5–75.6 | 15,017,305 |
| batch/omaha/5/3/1 | 37.0 | 32.9–38.9 | 27,039,873 |
| batch/omaha/5/3/1024 | 24.7 | 24.2–26.0 | 40,559,274 |
| batch/omaha/5/3/64 | 30.2 | 29.7–32.1 | 33,079,209 |
| batch/omaha/5/3/8 | 32.1 | 29.5–34.6 | 31,198,586 |
| batch/omaha/5/4/1 | 56.8 | 55.8–61.2 | 17,591,479 |
| batch/omaha/5/4/1024 | 54.3 | 45.2–60.8 | 18,425,883 |
| batch/omaha/5/4/64 | 55.3 | 47.9–77.9 | 18,089,955 |
| batch/omaha/5/4/8 | 55.2 | 50.4–65.1 | 18,129,028 |
| batch/omaha/5/5/1 | 93.2 | 87.9–117.8 | 10,731,728 |
| batch/omaha/5/5/1024 | 79.4 | 78.3–96.9 | 12,587,275 |
| batch/omaha/5/5/64 | 83.9 | 82.6–142.3 | 11,912,517 |
| batch/omaha/5/5/8 | 103.5 | 84.7–128.0 | 9,664,207 |
| batch/omaha/6/3/1 | 40.8 | 39.2–47.1 | 24,498,780 |
| batch/omaha/6/3/1024 | 31.0 | 29.8–34.3 | 32,222,537 |
| batch/omaha/6/3/64 | 36.5 | 33.9–49.5 | 27,416,332 |
| batch/omaha/6/3/8 | 38.1 | 36.1–45.5 | 26,260,450 |
| batch/omaha/6/4/1 | 66.0 | 65.1–88.7 | 15,146,585 |
| batch/omaha/6/4/1024 | 62.5 | 54.8–135.9 | 16,010,006 |
| batch/omaha/6/4/64 | 59.9 | 57.5–68.8 | 16,706,366 |
| batch/omaha/6/4/8 | 62.6 | 61.7–99.8 | 15,980,025 |
| batch/omaha/6/5/1 | 117.4 | 115.6–159.0 | 8,514,460 |
| batch/omaha/6/5/1024 | 113.9 | 100.5–124.5 | 8,781,634 |
| batch/omaha/6/5/64 | 106.3 | 103.1–160.6 | 9,411,419 |
| batch/omaha/6/5/8 | 112.8 | 105.0–157.5 | 8,869,180 |
| native/high/5 | 30.4 | 29.6–31.5 | 32,864,754 |
| native/high/6 | 70.4 | 67.9–93.4 | 14,195,409 |
| native/high/7 | 73.4 | 68.4–134.1 | 13,624,268 |
| parallel/high/7/1 | 175.5 | 158.4–194.3 | 5,698,513 |
| parallel/high/7/2 | 98.7 | 79.0–112.3 | 10,134,199 |
| parallel/high/7/4 | 89.8 | 74.1–132.7 | 11,138,668 |
| parallel/omaha/4/5/1 | 176.6 | 145.0–224.6 | 5,663,779 |
| parallel/omaha/4/5/2 | 127.5 | 120.1–132.4 | 7,842,897 |
| parallel/omaha/4/5/4 | 113.3 | 110.9–152.5 | 8,823,175 |
| prepared/high/2/1 | 50.0 | 48.2–57.9 | 20,009,379 |
| prepared/high/2/64 | 17.4 | 15.6–19.3 | 57,612,243 |
| prepared/high/2/8 | 22.8 | 22.5–29.1 | 43,940,954 |
| prepared/high/5/1 | 29.9 | 27.9–38.5 | 33,488,129 |
| prepared/high/5/64 | 13.1 | 10.2–28.1 | 76,497,834 |
| prepared/high/5/8 | 15.1 | 13.9–34.0 | 66,278,317 |
| prepared/high/6/1 | 28.2 | 27.7–35.9 | 35,415,370 |
| prepared/high/6/64 | 7.7 | 7.2–9.3 | 130,048,260 |
| prepared/high/6/8 | 14.7 | 11.9–16.1 | 67,859,509 |
| prepared/omaha/4/1 | 114.9 | 99.7–129.1 | 8,701,564 |
| prepared/omaha/4/128 | 57.5 | 53.8–79.4 | 17,394,259 |
| prepared/omaha/4/16 | 64.7 | 61.2–79.9 | 15,467,343 |
| prepared/omaha/5/1 | 113.7 | 103.6–149.0 | 8,797,402 |
| prepared/omaha/5/128 | 78.7 | 68.1–83.8 | 12,712,601 |
| prepared/omaha/5/16 | 81.8 | 77.7–89.7 | 12,225,698 |
| prepared/omaha/6/1 | 134.5 | 126.5–147.5 | 7,434,512 |
| prepared/omaha/6/128 | 96.0 | 89.0–111.1 | 10,416,667 |
| prepared/omaha/6/16 | 108.9 | 102.8–124.3 | 9,184,680 |
| scalar/high/5 | 69.9 | 68.3–72.5 | 14,310,870 |
| scalar/high/6 | 128.1 | 125.9–140.9 | 7,805,711 |
| scalar/high/7 | 94.8 | 91.7–180.7 | 10,552,132 |
| scalar/omaha/4/3 | 127.2 | 100.4–132.7 | 7,862,166 |
| scalar/omaha/4/4 | 53.9 | 51.3–55.4 | 18,556,103 |
| scalar/omaha/4/5 | 80.4 | 77.0–111.3 | 12,437,146 |
| scalar/omaha/5/3 | 45.8 | 44.4–50.6 | 21,848,596 |
| scalar/omaha/5/4 | 64.7 | 61.5–67.9 | 15,448,675 |
| scalar/omaha/5/5 | 113.4 | 96.0–125.5 | 8,821,654 |
| scalar/omaha/6/3 | 54.4 | 51.2–92.7 | 18,389,484 |
| scalar/omaha/6/4 | 73.0 | 71.5–128.6 | 13,693,501 |
| scalar/omaha/6/5 | 124.0 | 119.6–156.6 | 8,061,850 |
| shared/holdem/3/2 | 27.9 | 20.8–28.7 | 35,864,388 |
| shared/holdem/3/6 | 8.2 | 7.4–9.3 | 121,755,546 |
| shared/holdem/3/9 | 6.5 | 5.8–7.3 | 154,239,327 |
| shared/holdem/4/2 | 43.2 | 38.5–60.1 | 23,123,476 |
| shared/holdem/4/6 | 19.3 | 18.0–23.5 | 51,916,692 |
| shared/holdem/4/9 | 15.0 | 14.7–17.2 | 66,588,785 |
| shared/holdem/5/2 | 63.7 | 61.9–69.6 | 15,695,411 |
| shared/holdem/5/6 | 30.7 | 26.3–32.4 | 32,598,395 |
| shared/holdem/5/9 | 24.3 | 23.0–25.5 | 41,095,890 |
| shared/omaha/4/3/2 | 26.3 | 23.7–28.1 | 37,965,297 |
| shared/omaha/4/3/6 | 21.8 | 19.0–46.9 | 45,862,590 |
| shared/omaha/4/4/2 | 41.0 | 40.4–44.8 | 24,416,997 |
| shared/omaha/4/4/6 | 35.0 | 34.0–57.7 | 28,550,877 |
| shared/omaha/4/5/2 | 62.5 | 61.8–89.0 | 16,010,006 |
| shared/omaha/4/5/6 | 53.8 | 51.7–72.8 | 18,599,957 |
| shared/omaha/5/3/2 | 29.6 | 29.0–30.3 | 33,822,169 |
| shared/omaha/5/3/6 | 25.0 | 23.7–26.7 | 40,046,566 |
| shared/omaha/5/4/2 | 56.9 | 49.2–65.0 | 17,580,003 |
| shared/omaha/5/4/6 | 46.0 | 44.5–53.3 | 21,722,657 |
| shared/omaha/5/5/2 | 84.6 | 79.4–110.0 | 11,818,748 |
| shared/omaha/5/5/6 | 73.7 | 69.3–106.3 | 13,563,956 |
| shared/omaha/6/3/2 | 36.2 | 35.3–38.4 | 27,638,327 |
| shared/omaha/6/3/6 | 33.6 | 32.9–34.2 | 29,804,193 |
| shared/omaha/6/4/2 | 62.6 | 59.9–85.4 | 15,970,056 |
| shared/omaha/6/4/6 | 60.5 | 53.6–85.7 | 16,533,692 |
| shared/omaha/6/5/2 | 129.1 | 108.9–133.0 | 7,746,543 |
| shared/omaha/6/5/6 | 104.2 | 93.0–114.6 | 9,600,000 |
| simulation/holdem/2 | 201.7 | 195.0–211.0 | 4,957,253 |
| simulation/holdem/6 | 92.5 | 81.9–98.6 | 10,805,031 |
| simulation/omaha/2 | 202.9 | 197.2–207.7 | 4,928,574 |
| simulation/omaha/6 | 127.4 | 122.3–129.7 | 7,848,708 |
| startup/initialize | 270.0 | 230.0–380.0 | 3,703,704 |
| state/clone/high | 17.6 | 16.5–25.2 | 56,725,017 |
| state/clone/omaha | 17.5 | 16.8–19.2 | 57,098,249 |
| state/extend/high | 26.5 | 25.6–27.8 | 37,769,253 |
| state/extend/omaha | 28.5 | 27.6–30.4 | 35,097,340 |
| total/high/2/1 | 58.6 | 56.0–81.1 | 17,074,635 |
| total/high/2/64 | 16.3 | 15.2–21.5 | 61,200,096 |
| total/high/2/8 | 22.3 | 21.5–29.0 | 44,786,564 |
| total/high/5/1 | 47.8 | 45.7–51.5 | 20,918,451 |
| total/high/5/64 | 11.2 | 9.7–14.8 | 89,667,251 |
| total/high/5/8 | 16.4 | 14.5–19.4 | 60,843,731 |
| total/high/6/1 | 44.2 | 44.2–55.9 | 22,612,843 |
| total/high/6/64 | 8.9 | 7.7–14.9 | 112,329,969 |
| total/high/6/8 | 15.7 | 13.9–19.7 | 63,721,220 |
| total/omaha/4/1 | 106.2 | 96.6–141.3 | 9,415,053 |
| total/omaha/4/128 | 62.0 | 56.6–73.7 | 16,125,984 |
| total/omaha/4/16 | 66.0 | 64.0–74.2 | 15,160,039 |
| total/omaha/5/1 | 123.1 | 118.8–180.2 | 8,123,374 |
| total/omaha/5/128 | 73.0 | 70.8–75.1 | 13,701,196 |
| total/omaha/5/16 | 81.4 | 78.7–91.9 | 12,284,659 |
| total/omaha/6/1 | 149.6 | 143.5–161.2 | 6,684,684 |
| total/omaha/6/128 | 99.0 | 90.1–129.5 | 10,097,624 |
| total/omaha/6/16 | 111.9 | 100.7–124.9 | 8,932,622 |

See `latest.json` for seeds, hashes, resource observations and environment details; `history/` preserves previous snapshots. Raw measurements are retained as workflow artifacts.
