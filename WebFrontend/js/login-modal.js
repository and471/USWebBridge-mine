
function LoginModalController(container) {
	this.container = container;
	this.wrapper = this.container.parent(".modal-wrapper");
	this.passwordInput = this.container.find("input#login-password");
	this.button = this.container.find("button#login-submit");

	this.container.find("form").submit(this.onFormSubmit.bind(this));

	this.submitCallback = null;
}

LoginModalController.prototype.onFormSubmit = function(event) {
	event.preventDefault();
	if (this.submitCallback) this.submitCallback(this.passwordInput.val());
};

LoginModalController.prototype.submit = function(cb) {
	this.submitCallback = cb;
}
 
LoginModalController.prototype.fail = function() {
	this.button.removeClass("btn-primary").addClass("btn-danger").text("Failed");

	setTimeout(function() {
		this.button.removeClass("btn-danger").addClass("btn-primary").text("Submit");
	}.bind(this), 2000);
}

LoginModalController.prototype.success = function() {
	this.wrapper.fadeOut();
}

LoginModalController.prototype.show = function() {
	this.wrapper.fadeIn();
}