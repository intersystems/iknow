"""Calculate the order in which the jobs should be queued to optimize the Travis
build runtime.

Assumptions:
- Up to 5 jobs can run in parallel.
- If a new job is in the queue and fewer than 5 jobs are running, the new job
  starts with negligible delay.

Edit the build_times dictionary to indicate the average runtime of each job in
integer seconds, then execute this script to see the optimal ordering of the
jobs in .travis.yml."""

import itertools

build_times = {
    'manylinux2014_aarch64': 13*60+8,
    'manylinux2014_ppc64le': 12*60+2,
    'manylinux2014_x86_64': 14*60+3,
    'manylinux2014_i686': 12*60+46,
    'manylinux2010_x86_64': 12*60+21,
    'manylinux2010_i686': 13*60,
    'Mac OS X 10.9 x86_64': 20*60+6,
    'Windows x86_64': 12*60+33
}

QUEUE_LENGTH = 5


def runtime(permutation):
    """Calculate and return the estimated runtime of a given permutation of the
    job ordering. Also return a trace of how the jobs go through the pipeline."""
    queue = list(permutation)
    running = {x: build_times[x] for x in queue[:QUEUE_LENGTH]}
    del queue[:QUEUE_LENGTH]
    t = 0
    trace = [{**running}]
    while running:
        min_time = min(running.values())
        for job in running:
            running[job] -= min_time
        t += min_time
        trace.append({**running})
        trace.append('time elapsed: {}'.format(min_time))
        running = {k:v for k,v in running.items() if v > 0}
        trace.append({**running})
        if len(running) < QUEUE_LENGTH and queue:
            while len(running) < QUEUE_LENGTH and queue:
                running[queue[0]] = build_times[queue[0]]
                del queue[0]
            trace.append({**running})
    return t, trace


min_runtime = sum(build_times.values())
min_permutation = None
min_trace = None
for permutation in itertools.permutations(build_times):
    r, trace = runtime(permutation)
    if r < min_runtime:
        min_runtime = r
        min_permutation = permutation
        min_trace = trace

print('Optimal job order is')
for job in min_permutation:
    print('\t{}'.format(job))
print('\nExpected runtime trace is')
for line in min_trace:
    print('\t{}'.format(line))
print('\nApproximate runtime is {} min {} s.'.format(min_runtime // 60, min_runtime % 60))
