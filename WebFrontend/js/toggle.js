function ToggleButton(button, callback) {
    this.button = button;
    this.toggled = false;
    this.callback = callback;

    this.button.click(this.onToggle.bind(this));
}

ToggleButton.prototype.onToggle = function() {
    this.toggled = !this.toggled;
    this.button.toggleClass("btn-orange").toggleClass("btn-dark");
    this.callback(this.toggled);
}

