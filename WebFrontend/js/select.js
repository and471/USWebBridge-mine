/* Selection handles

        1
    0 ------ 2
    7 |    | 3
    6 ------ 4
       5
*/

MOVE = 0;
RESIZE = 1;

NW = 0;
N  = 1;
NE = 2;
E  = 3;
SE = 4;
S  = 5;
SW = 6;
W  = 7;


function isNear(x, y, x1, y1, tolerance, handle) {
    var diffX, diffY;

    if (handle == 0 || handle == 7 || handle == 6) diffX = x - x1;
    if (handle == 2 || handle == 3 || handle == 4) diffX = x1 - x;

    if (handle == 1 || handle == 5) diffX = Math.abs((x - x1) * 2);

    if (handle == 0 || handle == 1 || handle == 2) diffY = y - y1;
    if (handle == 4 || handle == 5 || handle == 6) diffY = y1 - y;

    if (handle == 3 || handle == 7) diffY = Math.abs((y - y1) * 2);

    return diffX <= tolerance && diffX > 0 &&
           diffY <= tolerance && diffY > 0;
}

function Selection(x1, y1, x2, y2, maxX, maxY) {
  this.x1 = x1;
  this.y1 = y1;
  this.x2 = x2;
  this.y2 = y2;
  this.BORDER_COLOURS = ["#999999", "#ebebeb", "#999999"];

  this.maxX = maxX;
  this.maxY = maxY;
}

Selection.prototype.drawBorder = function(ctx, x1, y1, x2, y2) {
    var i;
    for (i = 0; i < 3; i++) {
        ctx.fillStyle = this.BORDER_COLOURS[i];
        ctx.fillRect(x1 + i, y1 + i,
                     x2 - x1 - i * 2, y2 - y1 - i * 2);
    }

    ctx.save();
    ctx.fillStyle = 'white';
    ctx.globalCompositeOperation = 'destination-out';
    ctx.fillRect(x1 + i, y1 + i,
                 x2 - x1 - i * 2, y2 - y1 - i * 2);
    ctx.restore();
}

Selection.prototype.drawHandle = function(ctx, handle) {
    var handleSize = 20;
    var x,y, cover1X, cover1Y, cover2X, cover2Y;

    switch (handle) {
        case 0: 
                x = this.x1; y = this.y1;
                cover1X = x + 1; cover1Y = y + handleSize - 1;
                cover2X = x + handleSize - 1; cover2Y = y + 1;
                break;
        case 1: x = Math.floor((this.x1 + this.x2 - handleSize)/2); y = this.y1;
                cover1X = x; cover1Y = y + 1;
                cover2X = x + handleSize - 1; cover2Y = y + 1;
                break;
        case 2: x = this.x2 - handleSize; y = this.y1;
                cover1X = x; cover1Y = y + 1;
                cover2X = x + handleSize - 2; cover2Y = y + handleSize - 1;
                break;
        case 3: x = this.x2 - handleSize; y = Math.floor((this.y1 + this.y2 - handleSize)/2);
                cover1X = x + handleSize - 2; cover1Y = y;
                cover2X = x + handleSize - 2; cover2Y = y + handleSize - 1;
                break; 
        case 4: x = this.x2 - handleSize; y = this.y2 - handleSize;
                cover1X = x + handleSize - 2; cover1Y = y;
                cover2X = x; cover2Y = y + handleSize - 2;
                break;
        case 5: x = Math.floor((this.x1 + this.x2 - handleSize)/2); y = this.y2 - handleSize;
                cover1X = x; cover1Y = y + 1;
                cover2X = x + handleSize - 1; cover2Y = y + 1;
                break; 
        case 6: x = this.x1; y = this.y2 - handleSize;
                cover1X = x + 1; cover1Y = y;
                cover2X = x + handleSize - 1; cover2Y = y + handleSize - 2;
                break;
        case 7: x = this.x1; y = Math.floor((this.y1 + this.y2 - handleSize)/2);
                cover1X = x + 1; cover1Y = y;
                cover2X = x + 1; cover2Y = y + handleSize - 1;
                break; 
    }

    this.drawBorder(ctx, x, y, x + handleSize, y + handleSize);
    ctx.fillStyle = this.BORDER_COLOURS[1];
    ctx.fillRect(cover1X, cover1Y, 1, 1);
    ctx.fillRect(cover2X, cover2Y, 1, 1);
}

Selection.prototype.drawOutline = function(ctx, draw_handles) {
  var squareSize = 8;

  this.drawBorder(ctx, this.x1, this.y1, this.x2, this.y2);

  if (draw_handles) {
    for (var i = 0; i < 8; i++) {
        this.drawHandle(ctx, i);
    }
  }
}

Selection.prototype.move = function(x, y) {
    width = this.x2 - this.x1;
    height = this.y2 - this.y1;
    this.x1 = x; this.x2 = x + width;
    this.y1 = y; this.y2 = y + height;
}


Selection.prototype.containsPoint = function(x, y) {
    return (x >= this.x1 && x <= this.x2 &&
            y >= this.y1 && y<= this.y2);
}

Selection.prototype.drawArea = function(ctx) {
  ctx.fillRect(this.x1, this.y1, this.x2 - this.x1, this.y2 - this.y1);
}

Selection.prototype.getResizeHandle = function(x, y) {
    var tolerance = 20;

    if (isNear(x, y, this.x1, this.y1, tolerance, NW)) return NW;
    if (isNear(x, y, this.x2, this.y1, tolerance, NE)) return NE;
    if (isNear(x, y, (this.x1 + this.x2)/2, this.y1, tolerance, N) && x < this.x2) return N;

    if (isNear(x, y, this.x1, this.y2, tolerance, SW)) return SW;
    if (isNear(x, y, this.x2, this.y2, tolerance, SE)) return SE;
    if (isNear(x, y, (this.x1 + this.x2)/2, this.y2, tolerance, S) && x < this.x2) return S;

    if (isNear(x, y, this.x1, (this.y1 + this.y2)/2, tolerance, W) && y < this.y2) return W;
    if (isNear(x, y, this.x2, (this.y1 + this.y2)/2, tolerance, E) && y < this.y2) return E;
    return -1;
}



$.widget("widgets.questionselect", {

    options: {},

    selections : [],
    canvas : null,

    mousedown : false,
    dragXOffset : -1,
    dragYOffset : -1,
    dragDirection : -1,
    selected : null,
    show_regions : true,

    _create: function() {
        var that = this
        
        var background = "";
        this.height = 0;
        for (var img in this.options.imgs) {
            this.width = this.options.imgs[img].width; // all same width
            background += "url(" + img + ") 0px " + this.height + "px no-repeat," 
            this.height += this.options.imgs[img].height;
        }

        // Remove trailing comma
        background = background.slice(0, background.length - 1)

        this.element.addClass("widget-questionselector");
        this.canvas = $("<canvas></canvas>").attr({
            width : this.width,
            height : this.height
        }).mousemove(function(event) { that._on_mousemove(that, event); })
          .mousedown(function(event) { that._on_mousedown(that, event); })
          .mouseup(function(event) { that._on_mouseup(that, event); })
          .dblclick(function(event) { that._on_doubleclick(that, event); })
          .attr("tabindex", "0")
          .keydown(function(event) { that._on_keyup(that, event); }) 
          .bind('selectstart', function(e) { e.preventDefault(); return false; })
          .css("background", background)
          .appendTo(this.element).get(0);

        this.element.scrollable({margin:30});
        this._draw();
    },

    _draw: function() {
        var ctx = this.canvas.getContext('2d')
        ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);

        if (this.show_regions) {
            ctx.save();
            ctx.fillStyle = "rgba(0,0,0,0.9)";
            ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);

            ctx.globalCompositeOperation = 'destination-out';
            ctx.fillStyle = "white";
            for (var i = 0; i < this.selections.length; i++) {
                this.selections[i].drawArea(ctx);
            }
            ctx.restore();
        } else {
            ctx.save();
            for (var i = 0; i < this.selections.length; i++) {
                if (i != this.selected) this.selections[i].drawOutline(ctx, false);
            }
            ctx.restore()
        }
        ctx.save();
        if (this.selected != null) this.selections[this.selected].drawOutline(ctx, true);
        ctx.restore();
    },

    _on_keyup: function(that, event) {
        // Delete key
        if (event.keyCode == 46) this.removeSelection();
    }, 

    _on_mousemove: function(that, event) {
        this._fix_event(event);
        var i;
        var cursor = 'auto';
        var handle = -1;

        for (i = 0; i < that.selections.length; i++) {
            var handle = that.selections[i].getResizeHandle(event.offsetX, event.offsetY);
            if (handle != -1) break;
        }

        if (!that.mousedown) {
            switch (handle) {
              case NW: cursor='nw-resize'; break;
              case N: cursor='n-resize';  break;
              case NE: cursor='ne-resize'; break;
              case E: cursor='e-resize';  break;
              case SE: cursor='se-resize'; break;
              case S: cursor='s-resize';  break;
              case SW: cursor='sw-resize'; break;
              case W: cursor='w-resize';  break;
            }
            that.element.css("cursor", cursor);
        } else {

            // Create new on drag
            if (this.selected == null) {
                return;
            };

            if (that.dragAction == RESIZE) {
                if (that.dragDirection == 2 || that.dragDirection == 3 ||
                    that.dragDirection == 4)
                {
                    that.selections[that.selected].x2 = event.offsetX;
                }
                if (that.dragDirection == 4 || that.dragDirection == 5 ||
                    that.dragDirection == 6)
                {
                    that.selections[that.selected].y2 = event.offsetY;
                }
                if (that.dragDirection == 0 || that.dragDirection == 7 ||
                    that.dragDirection == 6)
                {
                    that.selections[that.selected].x1 = event.offsetX;
                }
                if (that.dragDirection == 0 || that.dragDirection == 1 ||
                    that.dragDirection == 2)
                {
                    that.selections[that.selected].y1 = event.offsetY;
                }

            } else if (that.dragAction == MOVE) {
                that.selections[that.selected].move(event.offsetX - this.dragXOffset,
                                                    event.offsetY - this.dragYOffset);
            }
            that._draw();
        }


    },

    _on_mousedown: function(that, event) {
        this._fix_event(event);
        that.mousedown = true;

        for (i = 0; i < that.selections.length; i++) {
            var handle = that.selections[i].getResizeHandle(event.offsetX, event.offsetY);
            if (handle != -1) {
                this.selected = i;
                this.dragDirection = handle;
                this.dragAction = RESIZE;
                return;
            };
        }

        // No handle selected, try to see if a selection is selected
        for (i = 0; i < that.selections.length; i++) {
            if (that.selections[i].containsPoint(event.offsetX, event.offsetY)) {
                this.selected = i;
                this.dragXOffset = event.offsetX - that.selections[i].x1,
                this.dragYOffset = event.offsetY - that.selections[i].y1,
                this.dragAction = MOVE;
                return;
            }
        }
    },

    _on_mouseup: function(that, event) {
        this._fix_event(event);
        that.mousedown = false;
        this.dragXOffset = this.dragYOffset = -1;

        that.dragDirection = -1;
        that.dragAction = -1;

        var withinSelection = false;
        for (var i = 0; i < this.selections.length; i++) {
            if (this.selections[i].containsPoint(event.offsetX, event.offsetY)) 
                withinSelection = true;
        }
        if (!withinSelection) that.selected = null;

        that._draw();
    },

    _on_doubleclick: function(that, event) {
        this._fix_event(event);
        this.addSelection(event.offsetX - 100/2, event.offsetY - 100/2);
    },


    getSelections: function() {
        return this.selections;
    },

    addSelection: function(x, y) {
        var default_size = 100;
        x = x || Math.floor((
            this.width - default_size)/2);
        y = y || Math.floor(
            this.element.scrollTop() + 
                ( parseInt(this.element.css('height')) - default_size  ) /2);
        this.selected = this.selections.push(
            new Selection(x, y, x+default_size, y+default_size)) - 1;
        this._draw();
    },

    removeSelection: function() {
        if (this.selected != null) {
            this.selections.splice(this.selected, 1);
            this.selected = null;
            this._draw();
        }
    },

    showRegions: function() {
        this.show_regions = true;
        this._draw();
    },

    hideRegions: function() {
        this.show_regions = false;
        this._draw();
    },

    _fix_event: function(event) {
        // For firefox from http://stackoverflow.com/questions/22716333/firefox-javascript-mousemove-not-the-same-as-jquery-mousemove
        if(typeof event.offsetX === "undefined" || 
           typeof event.offsetY === "undefined") {
           var targetOffset = $(event.target).offset();
           event.offsetX = event.pageX - targetOffset.left;
           event.offsetY = event.pageY - targetOffset.top;
        }
    }

});
