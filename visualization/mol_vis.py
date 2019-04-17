# Run this bad boy in interative mode

import sys
import argparse

import vispy
from vispy import scene
import numpy as np

class Updater:
    def __init__(self, frames, colors, points, cm):
        self.frames = frames
        self.colors = colors
        self.points = points
        self.cm = cm
        self.i = 1

    def update(self, ev):
        self.points.set_data(self.frames[self.i], face_color=self.cm.map(self.colors[self.i]))
        self.i += 1


if __name__ == '__main__':

    output_file = 'out.mp4'

    # Parsing arguments

    parser = argparse.ArgumentParser()

    parser.add_argument('frames', help='NumPy array of points coordinates.')
    parser.add_argument('colors', help='NumPy array of points velocities.')
    parser.add_argument('--fps', default=20, type=int)
    parser.add_argument('-r', '--realtime', default=True, type=int)
    parser.add_argument('--rotateangle', default=3., type=float)
    parser.add_argument('--cubesize', default=100, type=int)

    args = parser.parse_args()

    if args.frames is None or args.colors is None:
        raise Exception('No files provided')

    # Load and transform data

    frames = np.load( args.frames )
    colors = np.load( args.colors )

    colors /= colors.max()
    cm = vispy.color.Colormap(['yellow', 'orange', 'red'], [0, 0.5, 1])

    # Bureaucracy

    win = scene.SceneCanvas(keys='interactive',
                            size=(800, 800), show=True, bgcolor='white')

    view = win.central_widget.add_view()
    view.camera = 'arcball'

    coor_rang = np.abs(frames).max() * 1.3
    view.camera.set_range(x=[-coor_rang, coor_rang], y=[-coor_rang, coor_rang], z=[-coor_rang, coor_rang])

    # Real thing

    molecules = scene.visuals.Markers(parent=view.scene)
    molecules.set_data(frames[0], face_color=cm.map(colors[0]))

    cz = args.cubesize
    borders = scene.visuals.Cube((cz, cz, cz), color=[0.1, 0.1, 0.1, 0.3],
                               edge_color='black', parent=view.scene)

    ## Interactive animation
    if args.realtime:

        molecules.events.update.connect(lambda evt: win.update)

        upd = Updater(frames, colors, molecules, cm)

        timer = vispy.app.Timer()
        timer.connect(upd.update)
        timer.start(interval=1/args.fps)

        if sys.flags.interactive != 1:
            vispy.app.run()

    ## Video creation
    else:
        import imageio

        writer = imageio.get_writer(output_file, fps=args.fps)

        axis = [0, 0, 1]

        for i in range(1, frames.shape[0]):
            im = win.render()
            writer.append_data(im)

            view.camera.transform.rotate(args.rotateangle, axis)

            molecules.set_data(frames[i], face_color=cm.map(colors[i]))

writer.close()