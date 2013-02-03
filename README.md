# d2 - a deadlock detection framework using intrusive dynamic analysis

## How does it work?
By recording specific events during the execution of a program, we construct
a trace of the program. After the program has finished, we analyze the trace
that was generated, which allows us to discover interesting properties of the
program.

Right now, we can find deadlocks that did not happen during that run of the
program but that could have happened under different thread scheduling
conditions.

More specifically, the current implementation constructs graphs detailing the
order of locking inside a thread, the start and join dependencies between
threads and other properties. A traversal of the graph will then yield
diagnostics of potential deadlocks, which we will filter to reduce false
positives.

## Intrusive?
It means that the code will have to be modified (slightly) in order for the
framework to be notified on specific events. Typically, only classes
representing synchronization objects and threads will have to be modified.
A simple mutex class could look like that:

    struct mutex : d2::deadlock_detectable<mutex> {
        void lock() {
            // lock the underlying mutex like you would normally...

            // notify d2 of the event
            d2::notify_acquire(boost::this_thread::get_id(), *this);
        }

        void unlock() {
            // unlock the underlying mutex like you would normally...

            // notify d2 of the event
            d2::notify_release(boost::this_thread::get_id(), *this);
        }
    };

Of course, the framework can be disabled statically, in which case the
`d2::notify_*` calls are no-ops and the `d2::deadlock_detectable` base class
is optimized away.

### Rationale
We decided to use intrusive dynamic analysis because of its simplicity and
flexibility. Being intrusive is the only way we know to support custom
synchronization objects. Having tried other non-intrusive solutions, the
results were usually bad since the tools could not detect deadlocks involving,
for instance, our custom spin mutex using atomic flags.

## Disclaimer:
The algorithm for performing deadlock detection was adapted from an algorithm
formally described in a series of papers. While the algorithm has undergone
some modifications, we do not claim to have authored the original algorithm
in itself. The paper is _Detection of deadlock potentials in multithreaded
programs_ that appeared in the Sept.-Oct. 2010 issue of the _IBM Journal of
Research and Development_.
