from __future__ import print_function
import time
import spynnaker_mpi

ACTION_INIT = "init"
ACTION_START = "start"
ACTION_STOP = "stop"

binaries = {
    960: {
        10: {
            True: "../project_oganesson_bin/nbody_v4_960_10.aplx",
            False: "../project_oganesson_bin/nbody_v4_960_10_nocomm.aplx"},
        50: {
            True: "../project_oganesson_bin/nbody_v4_960_50.aplx",
            False: "../project_oganesson_bin/nbody_v4_960_50_nocomm.aplx",},
    },
    1920: {
        10: {
            True: "../project_oganesson_bin/nbody_v4_960_10.aplx",
            False: "../project_oganesson_bin/nbody_v4_960_10_nocomm.aplx"},
        50: {
            True: "../project_oganesson_bin/nbody_v4_960_50.aplx",
            False: "../project_oganesson_bin/nbody_v4_960_50_nocomm.aplx",},
    }
}


def get_objects(app, r, p,
                spin_url="spin5.polito.it",
                bmp_url="bmp-spin5.polito.it",
                version=5, verbose=False):

    runtime = spynnaker_mpi.Runtime(spin_url, bmp_url, version)
    if verbose:
        print(runtime)

    context = spynnaker_mpi.Context(runtime, r, p)
    if verbose:
        print(context)

    app = spynnaker_mpi.App(runtime, app)
    if verbose:
        print(app)

    return runtime, context, app


def do_action(runtime, context, app, action, verbose=False):
    if action == ACTION_INIT:
        app.init(context)
        time.sleep(1)
        if verbose:
            print(context)
        return

    if action == ACTION_START:
        app.run()
        time.sleep(1)
        return

    if action == ACTION_STOP:
        buffers = app.stop(get_buffers=True)
        s = spynnaker_mpi.Utils.get_io_buffers(buffers)

        minimum = float('inf')
        maximum = 0
        mean = 0
        i = 0

        for line in s:
            if "End:" in line:
                t = line.split(" ")[3]
                minimum = min(minimum, int(t))
                maximum = max(maximum, int(t))
                mean += int(t)
                i += 1.0

        time.sleep(1)
        if verbose:
            print(context)

        return maximum, minimum, mean


def start(app, r, p):
    print("  START")
    runtime, context, app = get_objects(app, r, p)

    print("    Phase1")
    do_action(runtime, context, app, ACTION_INIT, verbose=True)

    print("    Phase2")
    do_action(runtime, context, app, ACTION_START)

    print("    Phase3")
    maximum, minimum, mean = do_action(runtime, context, app, ACTION_STOP)

    del app
    del context
    del runtime

    return maximum, minimum, mean


def main():
    results = {}

    atoms = 1920
    nodes_max = 256
    nodes_min = 1

    if atoms == 960:
        app = "../project_oganesson_bin/nbody_v5_960_10.aplx"
    elif atoms == 1920:
        app = "../project_oganesson_bin/nbody_v5_1920_10.aplx"
    else:
        return

    for r in range(0, 8):
        for p in range(1, 17):
            nodes = spynnaker_mpi.Utils.spinn5_radius_processors(r, p)
            if nodes_min < nodes < nodes_max and atoms % nodes == 0:
                print(nodes, r, p)
                maximum, minimum, mean = start(app, r, p)
                print(maximum, minimum, mean)

                if nodes not in results.keys():
                    results[nodes] = dict()

                results[nodes][(r, p)] = (maximum, minimum, mean)
    print(results)
    nodes_list = sorted(results.keys())
    for nodes in nodes_list:
        pairs = sorted(results[nodes].keys())
        for pair in pairs:
            print(nodes, pair[0], pair[1], results[nodes][pair])


if __name__ == "__main__":
    main()