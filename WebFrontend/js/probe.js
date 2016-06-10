function rad(deg) { return deg * 2 * Math.PI / 360; }

function ProbeVisualisation(container, width, height) {
    this.enabled = Detector.webgl;
    if (!this.enabled) return;

    this.container = container;
    this.container.addClass("probe");

    this.width = width;
    this.height = height;

    // Create renderer and add canvas to page
    this.renderer = new THREE.WebGLRenderer({antialias : true});
    this.renderer.setSize(width, height);
    this.container.append($(this.renderer.domElement));
    this.overlay = $("<canvas></canvas>").addClass("overlay")
                                         .attr("width",  width).attr("height", height)
                                         .appendTo(container);
    this.context = this.overlay.get(0).getContext("2d");

    this.createScene();
    // Load probe from file
    var loader = new THREE.ColladaLoader();
    loader.load('probe.dae', function(result) {
        this.probe = result.scene;
        this.scene.add(this.probe);
    }.bind(this));

    this.render();
}

ProbeVisualisation.FORCE_COLORS = [
    "rgba(170, 185, 202, 0.85)", 
    "rgba(173, 197, 162, 0.85)",  
    "rgba(200, 183, 164, 0.85)"
];

ProbeVisualisation.prototype.createScene = function() {
    this.camera = new THREE.PerspectiveCamera(70, (this.width * 1) / this.height, 1,  200);
    this.camera.position.z = 5;

    this.scene = new THREE.Scene();

    // Create light sources
    var directionalLight = new THREE.AmbientLight(0xffffff, .3);
    this.scene.add(directionalLight);

    var directionalLight = new THREE.DirectionalLight(0xffffff, 1);
    directionalLight.position.set(1, 1, 1).normalize();
    this.scene.add(directionalLight);

    var directionalLight = new THREE.DirectionalLight(0xffffff,  .5);
    directionalLight.position.set(-1, -1, -1).normalize();
    this.scene.add(directionalLight);

    this.controls = new THREE.TrackballControls(this.camera, this.overlay.get(0));
    this.controls.rotateSpeed = 10.0;
    this.controls.noZoom = true;
    this.controls.noPan = true;
    this.controls.dynamicDampingFactor = 0.3;

};

ProbeVisualisation.prototype.render = function() {
    requestAnimationFrame(this.render.bind(this));

    this.controls.update();
    this.renderer.render(this.scene, this.camera);
}

ProbeVisualisation.prototype.setPositionOrientation = function(position, orientation) {
    if (!this.probe) {
        return;
    }

    // Position is [x, y, z],  -250 < X, Y < 250,  -500 < z < 0
    // Normalise to fit in a 8x8x8 cube with center (0, 0, 0)
    position[0] = (position[0] / 250) * 4;
    position[1] = (position[1] / 250) * 4;
    position[2] = ((position[2]+250)/250.) * 4;
    this.probe.position.set(position[0], position[1], position[2]);

    // Orientation is [x, y, z, w] where x, y, z give axis and w gives rotation about that axis
    this.probe.quaternion.set(orientation[0], orientation[1], orientation[2], orientation[3]);
}

ProbeVisualisation.prototype.setForces = function(forces) {
    if (!this.probe) {
        return;
    }
    this.forces = forces;

    // Normalise between +1 and -1
    this.forces[0] /= 20;
    this.forces[1] /= 20;
    this.forces[2] /= 20;

    this.drawForces();
}

ProbeVisualisation.prototype.drawForces = function() {
    this.context.clearRect(0, 0, this.width, this.height);

    for (var i = 0; i < 3; i++) {
        // Make force more visible, as max (20N) is rarely reached
        var force = logScale(logScale(logScale(this.forces[i])));

        var width = (this.width / 4) / 3;
        var height = Math.abs(force) * this.height/2;
        var x = this.width - (i+1) * width;

        var y = this.height / 2 - height;
        if (force < 0) {
            y = this.height / 2;
        }

        this.context.fillStyle = ProbeVisualisation.FORCE_COLORS[i];
        this.context.fillRect(x, y, width, height);
    }
}

function logScale(x, max) {
    // Takes a value from -1 to +1 and applies a log scale to it
    // Returns a value between -1 and +1
    var sign = (x >= 0) ? 1 : -1;

    return sign * (Math.log(Math.abs(x)+1) / Math.log(2));
}





/*function buildAxes( length ) {
        var axes = new THREE.Object3D();

        axes.add( buildAxis( new THREE.Vector3( 0, 0, 0 ), new THREE.Vector3( length, 0, 0 ), 0xFF0000, false ) ); // +X
        axes.add( buildAxis( new THREE.Vector3( 0, 0, 0 ), new THREE.Vector3( -length, 0, 0 ), 0xFF0000, true) ); // -X
        axes.add( buildAxis( new THREE.Vector3( 0, 0, 0 ), new THREE.Vector3( 0, length, 0 ), 0x00FF00, false ) ); // +Y
        axes.add( buildAxis( new THREE.Vector3( 0, 0, 0 ), new THREE.Vector3( 0, -length, 0 ), 0x00FF00, true ) ); // -Y
        axes.add( buildAxis( new THREE.Vector3( 0, 0, 0 ), new THREE.Vector3( 0, 0, length ), 0x0000FF, false ) ); // +Z
        axes.add( buildAxis( new THREE.Vector3( 0, 0, 0 ), new THREE.Vector3( 0, 0, -length ), 0x0000FF, true ) ); // -Z

        return axes;

}

function buildAxis( src, dst, colorHex, dashed ) {
        var geom = new THREE.Geometry(),
            mat; 

        if(dashed) {
                mat = new THREE.LineDashedMaterial({ linewidth: 3, color: colorHex, dashSize: 3, gapSize: 3 });
        } else {
                mat = new THREE.LineBasicMaterial({ linewidth: 3, color: colorHex });
        }

        geom.vertices.push( src.clone() );
        geom.vertices.push( dst.clone() );
        geom.computeLineDistances(); // This one is SUPER important, otherwise dashed lines will appear as simple plain lines

        var axis = new THREE.Line( geom, mat, THREE.LinePieces );

        return axis;

}*/