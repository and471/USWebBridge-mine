function rad(deg) { return deg * 2 * Math.PI / 360; }

function ProbeVisualisation(container, width, height) {
    this.container = container;
    this.container.addClass("probe");

    this.width = width;
    this.height = height;

    this.createScene();

    // Load probe from file
    var loader = new THREE.ColladaLoader();
    loader.load('probe.dae', function(result) {
        this.probe = result.scene;
        this.scene.add(this.probe);
        this.zoomToObject();
    }.bind(this));

    // Create renderer and add canvas to page
    this.renderer = new THREE.WebGLRenderer({antialias : true});
    this.renderer.setSize(width, height);
    this.container.append($(this.renderer.domElement));

    this.animate();
}


ProbeVisualisation.prototype.createScene = function() {
    // You can adjust the cameras distance and set the FOV to something
    // different than 45°. The last two values set the clippling plane.
    this.camera = new THREE.PerspectiveCamera( 45, this.width / this.height, 1, 2000);
    this.camera.position.z = 50;

    this.scene = new THREE.Scene();

    // Create light sources
    var directionalLight = new THREE.AmbientLight( 0xffffff , .3);
    this.scene.add( directionalLight );

    var directionalLight = new THREE.DirectionalLight( 0xffffff , 1);
    directionalLight.position.set( 1, 1, 1 ).normalize();
    this.scene.add( directionalLight );

    var directionalLight = new THREE.DirectionalLight( 0xffffff, .5 );
    directionalLight.position.set( -1, -1, -1 ).normalize();
    this.scene.add( directionalLight );
};

ProbeVisualisation.prototype.animate = function() {
    requestAnimationFrame(this.animate.bind(this));

    if (this.probe) {
        this.probe.rotateX(rad(1/4));
        this.probe.rotateY(rad(2/4));
        this.probe.rotateZ(rad(3/4));
    }

    this.renderer.render(this.scene, this.camera);
}

ProbeVisualisation.prototype.zoomToObject = function() {
    // create an helper
    var helper = new THREE.BoundingBoxHelper(this.probe);
    helper.update();
    // get the bounding sphere
    var boundingSphere = helper.box.getBoundingSphere();
    // calculate the distance from the center of the sphere
    // and subtract the radius to get the real distance.
    var center = boundingSphere.center;
    var radius = boundingSphere.radius;
    var distance = center.distanceTo(this.camera.position) - radius;
    var realHeight = Math.abs(helper.box.max.y - helper.box.min.y);
    var fov = 2 * Math.atan(realHeight * 1.3 / ( 2 * distance )) * ( 180 / Math.PI );
    this.camera.fov = fov;
    this.camera.updateProjectionMatrix();
}

ProbeVisualisation.prototype.setPositionOrientation = function(position, orientation) {

    // TODO

}