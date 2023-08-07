function quadcopterData() {
    const Movement = {
        UP: 'UP',
        DOWN: 'DOWN',
        UPDOWN: 'UPDOWN',
        LEFT: 'LEFT',
        RIGHT: 'RIGHT',
        FORWARD: 'FORWARD',
        BACKWARD: 'BACKWARD',
    };
    const Motor = {
        M1: {
            label: 'M1',
            position: {
                x: 0,
                y: 0,
                z: 0,
            },
            rotation: RotationDirection.COUNTERCLOCKWISE,
        },
        M2: {
            label: 'M2',
            position: {
                x: 0,
                y: 0,
                z: 0,
            },
            rotation: RotationDirection.CLOCKWISE,
        },
        M3: {
            label: 'M3',
            position: {
                x: 0,
                y: 0,
                z: 0,
            },
            rotation: RotationDirection.COUNTERCLOCKWISE,
        },
        M4: {
            label: 'M4',
            position: {
                x: 0,
                y: 0,
                z: 0,
            },
            rotation: RotationDirection.CLOCKWISE,
        },
    };
    return {
        Movement,
        Motor,
    };
}

