from __future__ import print_function
import time
import spynnaker_mpi
import spynnaker_acp
import spynnaker_acp.types

ACTION_INIT = "init"
ACTION_START = "start"
ACTION_STOP = "stop"

binaries = {3: {10: {True: "../project_oganesson_bin/nbody_v3_960_10.aplx",
                     False: "../project_oganesson_bin/nbody_v3_960_10_nocomm.aplx"},
                50: {True: "../project_oganesson_bin/nbody_v3_960_50.aplx",
                     False: "../project_oganesson_bin/nbody_v3_960_50_nocomm.aplx"}},
            4: {10: {True: "../project_oganesson_bin/nbody_v4_960_10.aplx",
                     False: "../project_oganesson_bin/nbody_v4_960_10_nocomm.aplx"},
                50: {True: "../project_oganesson_bin/nbody_v4_960_50.aplx",
                     False: "../project_oganesson_bin/nbody_v4_960_50_nocomm.aplx",
                     "nosync": "../project_oganesson_bin/nbody_v4_960_50_nosync.aplx"}},
            }


def get_objects(verbose=False):
    runtime = spynnaker_mpi.Runtime("spin5.polito.it", "bmp-spin5.polito.it", 5)
    if verbose:
        print(runtime)

    context = spynnaker_mpi.Context(runtime, 4, 4)

    if verbose:
        print(context)

    # app = spynnaker_mpi.App(runtime, binaries[4][10][True])
    # app = spynnaker_mpi.App(runtime, "../project_oganesson_bin/nbody_v5_960_10a.aplx")
    app = spynnaker_mpi.App(runtime, "mpi_nbody.aplx")
    if verbose:
        print(app)

    return runtime, context, app


def do_action(runtime, context, app, action, verbose=False):
    if action == ACTION_INIT:
        app.init(context)
        time.sleep(1)
        if verbose:
            print(context)

    if action == ACTION_START:
        app.run()

        # test_read = False

        # # ---> Test read
        # if test_read:
        #     x, y, p = context.get_processor(24)
        #     rank = spynnaker_acp.types.ACPIntegerType(32, False)
        #     cmd_read_value = spynnaker_acp.ACPImmediateCommand(
        #         spynnaker_acp.ACP_VAR_READ, spynnaker_mpi.Context.ACP_VARCODE_RANK, data=rank)
        #     pkt = spynnaker_acp.ACPPacket(x, y, p, cmd_read_value)
        #     runtime.send_sdp(pkt.sdp)
        # # <---

        time.sleep(1)

    if action == ACTION_STOP:
        buffers = app.stop(get_buffers=True)
        s = spynnaker_mpi.Utils.get_io_buffers(buffers)
        # s = s.split('\n')

        minimum = float('inf')
        maximum = 0
        mean = 0
        i = 0

        for line in s:
            if "End:" in line:
                t = line.split(" ")[3]
                # print time
                # print>>fileobj_w, time
                minimum = min(minimum, int(t))
                maximum = max(maximum, int(t))
                mean += int(t)
                i += 1.0

        print(mean/i, maximum/5.0, minimum/5.0, maximum-minimum)

        time.sleep(1)
        if verbose:
            print(context)


def main():
    runtime, context, app = get_objects()
    print("Context of %d nodes\n" % (context.cores_number,))

    print("Press a key to continue...")
    raw_input()
    do_action(runtime, context, app, ACTION_INIT, verbose=True)

    print("Press a key to continue...")
    raw_input()
    do_action(runtime, context, app, ACTION_START)

    print("Press a key to continue...")
    raw_input()
    do_action(runtime, context, app, ACTION_STOP)


if __name__ == "__main__":
    main()
