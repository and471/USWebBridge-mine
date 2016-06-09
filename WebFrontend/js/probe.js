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

    this.createScene();
    // Load probe from file
    var loader = new THREE.ColladaLoader();
    loader.load('probe.dae', function(result) {
        this.probe = result.scene;
        this.scene.add(this.probe);
    }.bind(this));

    this.animate();
}


ProbeVisualisation.prototype.createScene = function() {
    this.camera = new THREE.PerspectiveCamera(70, this.width / this.height, 1,  200);
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


    this.controls = new THREE.TrackballControls(this.camera, this.renderer.domElement);
    this.controls.rotateSpeed = 10.0;
    this.controls.noZoom = true;
    this.controls.noPan = true;
    this.controls.dynamicDampingFactor = 0.3;

};

ProbeVisualisation.prototype.animate = function() {
    requestAnimationFrame(this.animate.bind(this));

    this.controls.update();
    this.renderer.render(this.scene, this.camera);
}

ProbeVisualisation.prototype.setPositionOrientation = function(position, orientation) {
    if (!this.probe) {
        return;
    }

    // Position is [x, y, z]
    // Orientation is [x, y, z, w] where x, y, z give axis and w gives rotation about that axis

    console.log(position)

    //this.probe.position.set(position[0]/100, position[1]/100, position[2]/100);
    this.probe.quaternion.set(orientation[0], orientation[1], orientation[2], orientation[3]);
}


/* TODO:  REMOVE
function buildAxes( length ) {
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