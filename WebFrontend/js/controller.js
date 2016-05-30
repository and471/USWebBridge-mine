
$(document).ready(function() {
	controller = new Controller();
});

function Controller() {
	this.login_modal_controller = new LoginModalController($("#login-modal"));
	this.login_modal_controller.submit(this.onLoginSubmit.bind(this));
	
	this.RETRY_DELAY = 2000;
	Janus.init({debug: "all", callback: this.onJanusInit.bind(this)});
}

Controller.prototype.onLoginSubmit = function(password) {
	this.plugin.send({"message": {
		"request": "watch", 
		"auth": {
			"secret": password
		}
	}});
}

Controller.prototype.initUI = function() {
	this.snapshot_controller = new SnapshotController();
	$("#take-snapshot").click(this.onTakeSnapshotClicked.bind(this));


	$("#details-toggle").click(this.toggleDetails.bind(this));
	$("#send").click(function() {
		this.sendData.apply(this, [$('#datasend').val()])
	}.bind(this));

	this.sliceControl = new RangeValueControl($("#slice-control"), "3D Slice");
	this.sliceControl.change(this.onSliceChanged.bind(this));

	this.zoomControl = new RangeControl($("#zoom-control"), "Zoom");
	this.zoomControl.setMinMax(0.5, 4);
	this.zoomControl.setStep(0.1);
	this.zoomControl.setVal(1);
	this.zoomControl.change(this.onZoomChanged.bind(this));

	this.probe = new ProbeVisualisation($("#probe"), parseInt($("#probe").css("width"), 10), 250);
}

Controller.prototype.onJanusInit = function() {
	// Make sure the browser supports WebRTC
	if(!Janus.isWebrtcSupported()) {
		Janus.error("No WebRTC support... ");
		return;
	}

	var server = null;
	if(window.location.protocol === 'http:')
		server = "http://" + window.location.hostname + ":8088/janus";
	else
		server = "https://" + window.location.hostname + ":8089/janus";

	// Create session
	this.janus = new Janus({
		server: server,
		success: this.onSessionSuccess.bind(this),
		error: function(error) {
			Janus.error(error);
			Janus.error("Retrying in " + this.RETRY_DELAY + " milliseconds");
			setTimeout(function() {
				this.onJanusInit();
			}.bind(this), this.RETRY_DELAY);
			this.janus.destroy();
		}.bind(this),
		destroyed: function() {
			window.location.reload();
		}
	});

};
 
Controller.prototype.onSessionSuccess = function() {
	// Attach to ultrasound plugin
	this.janus.attach(
		{
			plugin: "plugin.ultrasound",
			success: this.onPluginSuccess.bind(this),
			error: function(error) {
				Janus.error("  -- Error attaching plugin... ", error);
			},
			onmessage: this.onPluginMessage.bind(this),
			ondataopen: function(data) {
				this.plugin.send({"message": {"request": "ready"}});
				$("#main").removeClass("disabled");
			}.bind(this),
			ondata: this.onData.bind(this),
			onremotestream: function(stream) {
				attachMediaStream($('#video').get(0), stream);
			},
			oncleanup: function() {}
		});
};

Controller.prototype.onPluginSuccess = function(plugin) {
	this.plugin = plugin;
	Janus.log("Plugin attached! (" + plugin.getPlugin() + ", id=" + plugin.getId() + ")");

	this.startStream();
}

Controller.prototype.onPluginMessage = function(msg, jsep) {
	if (msg["error"] !== undefined && msg["error"] !== null) {

		if (msg["error"] == "AUTH_FAIL") {
			this.login_modal_controller.fail();
			return;
		}

		stopStream();
		return;
	}

	var result = msg["result"];
	if (result !== null && result !== undefined && result["status"] !== undefined && result["status"] !== null) {
		this.handleStatus(result["status"]);
	} 
	if (jsep !== undefined && jsep !== null) {
		this.handleJSEP(jsep);
	}
}

Controller.prototype.onData = function(data_str) {
	var data = JSON.parse(data_str);

	if (data.method) {

		if (data.method === "NEW_PATIENT_METADATA") this.onNewPatientMetadata(data["data"]);
		if (data.method === "NEW_IMAGE_METADATA") this.onNewImageMetadata(data["data"]);
		if (data.method === "N_SLICES_CHANGED") this.onNSlicesChanged(data["data"]["nSlices"]);

	}
}


Controller.prototype.startStream = function() {
	this.login_modal_controller.show();
}

Controller.prototype.stopStream = function() {
	this.plugin.send({"message": {"request": "stop"}});
	this.plugin.hangup();
	this.janus.destroy();
}

Controller.prototype.handleStatus = function(status) {
	if (status === 'preparing') {
		// Means authentication succeeded
		this.login_modal_controller.success();
		this.initUI();
	} else if (status === 'starting') { 
		$('#video-status').removeClass('hide').text("Starting...").show();
	} else if (status === 'started') {
		$('#video-status').addClass('hide').show();
	} else if (status === 'stopped') {
		this.stopStream();
	}
}

Controller.prototype.handleJSEP = function(jsep) {
	// Answer
	this.plugin.createAnswer(
		{
			jsep: jsep,
			media: { audioSend: false, videoSend: false, data: true },	// We want recvonly audio/video
			success: function(jsep) {
				this.plugin.send({"message": {"request": "start"}, "jsep": jsep});
			}.bind(this),
			error: function(error) {
				Janus.error("WebRTC error:", error);
			}
		});
}

Controller.prototype.sendData = function(data) {
	this.plugin.data({
		text: JSON.stringify(data),
		error: function(reason) { bootbox.alert(reason); },
		success: function() { $('#datasend').val(''); },
	});
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
	this.sendData({"method": "SET_SLICE", "data":{"slice": slice}});
}

Controller.prototype.onZoomChanged = function() {
	var zoom = this.zoomControl.val();
	$("#video").css("transform", "scale(" + zoom + ")");
}

Controller.prototype.onTakeSnapshotClicked = function() {
	var img = this.snapshot_controller.snapshot($('#video').get(0), $('#video').get(0).videoWidth, $('#video').get(0).videoHeight);

	var container = $("<div></div>").addClass("snapshot-container")
	                                .append(img);

	img.click(function() {
		var a = $("<a></a>").attr("href", img.data("dataURL"))
					        .attr("download", img.data("filename"));
		a.get(0).click();
	})

	$("#snapshots-container").append(container);
}