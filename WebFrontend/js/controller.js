
$(document).ready(function() {
	controller = new Controller();
});


function Controller() {
	this.video = new VideoController($("#video-container"));
	this.video.crop(this.onCrop.bind(this));

	this.webrtc = new WebRTCController();
	this.webrtc.newImageMetadata(this.onNewImageMetadata.bind(this));
	this.webrtc.newPatientMetadata(this.onNewPatientMetadata.bind(this));
	this.webrtc.nSlicesChanged(this.onNSlicesChanged.bind(this));
	this.webrtc.authenticationSuccess(this.onAuthenticationSuccess.bind(this));
	this.webrtc.pluginSuccess(this.onPluginSuccess.bind(this));
	this.webrtc.remoteStream(this.video.onRemoteStream.bind(this.video));

	this.webrtc.init();

	this.login_modal = new LoginModalController($("#login-modal"));
	this.login_modal.submit(this.onLoginSubmit.bind(this));

	this.login_modal.show();
}

Controller.prototype.onLoginSubmit = function(password) {
	this.webrtc.watch({"secret": password});
}

Controller.prototype.initUI = function() {
	this.snapshot_controller = new SnapshotController();
	$("#take-snapshot").click(this.onTakeSnapshotClicked.bind(this));

	this.sliceControl = new RangeValueControl($("#slice-control"), "3D Slice");
	this.sliceControl.change(this.onSliceChanged.bind(this));

	this.zoomControl = new RangeControl($("#zoom-control"), "Zoom");
	this.zoomControl.setMinMax(0.5, 4);
	this.zoomControl.setStep(0.1);
	this.zoomControl.setVal(1);
	this.zoomControl.change(this.onZoomChanged.bind(this));

	this.probe = new ProbeVisualisation($("#probe"), parseInt($("#probe").css("width"), 10), 200);

	$("#freeze").click(this.togglePause.bind(this));
	$("#enhance-region").click(function() {
		this.video.showAlert("Select a region below to enhance. Unselected areas will not be updated.",  5000);
		this.video.enableCrop();
	}.bind(this));

	this.paused = false;
}

Controller.prototype.toggleDetails = function() {
	$("#video-col").toggleClass("col-md-12").toggleClass("col-md-8");
	$("#details-col").toggle();
}

Controller.prototype.onNewPatientMetadata = function(patient) {
	$("#detail-patient-name").text(patient["name"]);
}

Controller.prototype.onNewImageMetadata = function(metadata) {
	this.probe.setPositionOrientation(metadata["position"], metadata["orientation"]);
}

Controller.prototype.onNSlicesChanged = function(nSlices) {
	this.sliceControl.setVisible(nSlices > 1);

	this.sliceControl.setMinMax(-1*Math.floor(nSlices/2), Math.floor(nSlices/2));
}

Controller.prototype.onSliceChanged = function() {
	var slice = this.sliceControl.val();
	this.webrtc.sendData({"method": "SET_SLICE", "data":{"slice": slice}});
}

Controller.prototype.onZoomChanged = function() {
	this.video.zoom(this.zoomControl.val());
}

Controller.prototype.togglePause = function() {
	this.webrtc.togglePause();
	if (this.paused) {
		this.video.play();
		$("#freeze").text("Freeze");
	} else {
		this.video.pause();
		$("#freeze").text("Unfreeze");
	}

	$("#freeze").toggleClass("btn-dark").toggleClass("btn-default");
	this.paused = !this.paused;
}

Controller.prototype.onTakeSnapshotClicked = function() {
	var img = this.snapshot_controller.snapshot(this.video);

	var container = $("<div></div>").addClass("snapshot-container")
	                                .append(img);

	img.click(function() {
		var a = $("<a></a>").attr("href", img.data("dataURL"))
					        .attr("download", img.data("filename"));
		a.get(0).click();
	})

	$("#snapshots-container").append(container);
}

Controller.prototype.onAuthenticationSuccess = function() {
	this.login_modal.success();
	this.initUI();

}

Controller.prototype.onPluginSuccess = function() {
	this.login_modal.enable();
}

Controller.prototype.onCrop = function(selection) {
	this.webrtc.sendData({"method": "CROP", "data": {
		"left": selection.left(), "right": selection.right(), "top": selection.top(), "bottom": selection.bottom()
	}});
}