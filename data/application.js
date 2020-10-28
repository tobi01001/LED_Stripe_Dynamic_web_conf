// used when hosting the site on the ESP8266
var address = location.hostname;
var urlBase = "";

// used when hosting the site somewhere other than the ESP8266 (handy for testing without waiting forever to upload to SPIFFS)
//var address = "192.168.2.15";
//var urlBase = "http://" + address + "/";

var postColorTimer = {};
var postValueTimer = {};
var WSActiveTimer = {};
var WSInactiveTimer = {};
var statusActiveTimer = {};


var classSection = "default"; // for foldable sections
var firstSection = true;

const fieldtype = Object.freeze({
  NumberFieldType  : 0,
  BooleanFieldType : 1,
  SelectFieldType  : 2,
  ColorFieldType   : 3,
  TitleFieldType   : 4,
  SectionFieldType : 5,
  InvalidFieldType : 6
});

var DEBUGME = true; //for console loggin. 

var ignoreColorChange = false;

var ws_connected = false;

if(DEBUGME) console.log("Trying to connect to " + "ws://" + address + "/ws");
var ws = new ReconnectingWebSocket('ws://' + address + '/ws', ['arduino']);
ws.debug = true;

ws.onopen = function(evt) {
	if(evt.data !== null)
	{
		if(DEBUGME) console.log("Connection OPENED " + evt + " with " + evt.data);
	}
	ws_connected = true;
	updateWSState(true);
}

ws.onerror = function(evt) {
	if(evt.data !== null)
	{
		if(DEBUGME) console.log("ERROR: " + evt + " with " + evt.data);
	}
}

ws.onclose = function(evt) {
	if(evt.data !== null)
	{
		if(DEBUGME) console.log("Connection CLOSED " + evt + " with " + evt.data);
	}
	ws_connected = false;
	updateWSState(false);
}

ws.onmessage = function(evt) {

  if(evt.data !== null)
  {
    // added exception handling fro wrong json strings
    //if(DEBUGME) console.log("Received: " + evt + " with " + evt.data);
    var data = null;
	  try {
      data = JSON.parse(evt.data);
    } catch (e) {
		  console.log("Error " + e + " while decoding " + evt.data);
      return;
    }
	if(data.name != undefined)
	{
		if(DEBUGME) console.log("Received field data for field " + data.name + " with " + data.value);
		updateFieldValue(data.name, data.value);
		
	} else if (data.Client != undefined) {
		if(DEBUGME) console.log("Received Client info with from ID " + data.Client + ", Status: " + data.Status + ", Ping: " + data.Ping + ", Pong: " + data.Pong);
	} else {
		if(DEBUGME) console.log("Decoded: " + data + " with " + Object.values(data));		
	}
  }
  updateWSState(true);
}


function updateWSState(active)
{
	if(!active)
	{
		$("#WSAlive").html("<span class=\"dot\" style=\"background-color: #b00\"></span> WS dead");
		return;
	}
	$("#WSAlive").html("<span class=\"dot\" style=\"background-color: #0b0\"></span> WS alive" );
	clearTimeout(WSActiveTimer);
	WSActiveTimer = setTimeout(function() {
		$("#WSAlive").html("<span class=\"dot\" style=\"background-color: #bbb\"></span> WS alive");
	}, 1800);
	clearTimeout(WSInactiveTimer);
	WSInactiveTimer = setTimeout(function() {
		$("#WSAlive").html("<span class=\"dot\" style=\"background-color: #b00\"></span> WS dead");
	}, 2100);
}

function updateStatus(newStatus)
{
	$("#status").html(newStatus);
	clearTimeout(statusActiveTimer);
	statusActiveTimer = setTimeout(function() {
		$("#status").html("Ready");
	}, 4000);
}

$(document).ready(function() {
 updateStatus("Connecting, please wait...");

  $.get(urlBase + "/all", function(data) {
      updateStatus("Loading, please wait...");

      $.each(data, function(index, field) {
        //if(DEBUGME) console.log("Field " + field.label + " with type " + field.type);
        if (field.type == fieldtype.NumberFieldType) {
          addNumberField(field);
        } else if (field.type == fieldtype.TitleFieldType) {
          if (document.title != field.label) {
            document.title = field.label;
			$("#nameLink").html(field.label);
          }
        } else if (field.type == fieldtype.BooleanFieldType) {
          addBooleanField(field);
        } else if (field.type == fieldtype.SelectFieldType) {
          addSelectField(field);
        } else if (field.type == fieldtype.ColorFieldType) {
          // addColorFieldPalette(field); // removed this to save space on the page. no need currently
          addColorFieldPicker(field);
        } else if (field.type == fieldtype.SectionFieldType) {
          addSectionField(field);
        }
      });

      $(".minicolors").minicolors({
        theme: "bootstrap",
        changeDelay: 200,
        control: "brightness",  // changed to sqare one with brightness to the side
        format: "rgb",
        inline: true,
        swatches: ["FF0000", "FF8000", "FFFF00", "00FF00", "00FFFF", "0000FF", "FF00FF", "FFFFFF"] // some colors from the previous list
      });

      updateStatus("Ready");
    })
    .fail(function(errorThrown) {
      console.log("error: " + errorThrown);
    });
});

function addNumberField(field) {
  var template = $("#numberTemplate").clone();

  template.attr("id", "form-group-" + field.name);
  template.attr("data-field-type", field.type);
  template.addClass(classSection); // foldable sections
  if(firstSection) template.addClass("in");

  var label = template.find(".control-label");
  label.attr("for", "input-" + field.name);
  label.text(field.label);

  var input = template.find(".input");
  var slider = template.find(".slider");
  slider.attr("id", "input-" + field.name);
  if (field.min) {
    input.attr("min", field.min);
    slider.attr("min", field.min);
  }
  if (field.max) {
    input.attr("max", field.max);
    slider.attr("max", field.max);
  }
  if (field.step) {
    input.attr("step", field.step);
    slider.attr("step", field.step);
  }
  input.val(field.value);
  slider.val(field.value);

  slider.on("change mousemove", function() {
    input.val($(this).val());
  });

  slider.on("change", function() {
    var value = $(this).val();
    input.val(value);
    field.value = value;
    delayPostValue(field.name, value);
  });

  input.on("change", function() {
    var value = $(this).val();
    slider.val(value);
    field.value = value;
    delayPostValue(field.name, value);
  });

  $("#form").append(template);
}

function addBooleanField(field) {
  var template = $("#booleanTemplate").clone();

  template.attr("id", "form-group-" + field.name);
  template.attr("data-field-type", field.type);
  template.addClass(classSection); // foldable sections
  if(firstSection) template.addClass("in");

  var label = template.find(".control-label");
  label.attr("for", "btn-group-" + field.name);
  label.text(field.label);

  var btngroup = template.find(".btn-group");
  btngroup.attr("id", "btn-group-" + field.name);

  var btnOn = template.find("#btnOn");
  var btnOff = template.find("#btnOff");

  btnOn.attr("id", "btnOn" + field.name);
  btnOff.attr("id", "btnOff" + field.name);

  btnOn.attr("class", field.value ? "btn btn-primary" : "btn btn-default");
  btnOff.attr("class", !field.value ? "btn btn-primary" : "btn btn-default");

  btnOn.click(function() {
    setBooleanFieldValue(field, btnOn, btnOff, 1)
  });
  btnOff.click(function() {
    setBooleanFieldValue(field, btnOn, btnOff, 0)
  });

  $("#form").append(template);
}

function addSelectField(field) {
  var template = $("#selectTemplate").clone();

  template.attr("id", "form-group-" + field.name);
  template.attr("data-field-type", field.type);
  template.addClass(classSection); // foldable sections
  if(firstSection) template.addClass("in");

  var id = "input-" + field.name;

  var label = template.find(".control-label");
  label.attr("for", id);
  label.text(field.label);

  var select = template.find(".form-control");
  select.attr("id", id);

  for (var i = 0; i < field.options.length; i++) {
    var optionText = field.options[i];
    var option = $("<option></option>");
    option.text(optionText);
    option.attr("value", i);
    select.append(option);
  }

  select.val(field.value);

  select.change(function() {
    var value = template.find("#" + id + " option:selected").index();
    postValue(field.name, value);
  });

  var previousButton = template.find(".btn-previous");
  var nextButton = template.find(".btn-next");

  previousButton.click(function() {
    var value = template.find("#" + id + " option:selected").index();
    var count = select.find("option").length;
    value--;
    if(value < 0)
      value = count - 1;
    select.val(value);
    postValue(field.name, value);
  });

  nextButton.click(function() {
    var value = template.find("#" + id + " option:selected").index();
    var count = select.find("option").length;
    value++;
    if(value >= count)
      value = 0;
    select.val(value);
    postValue(field.name, value);
  });

  $("#form").append(template);
}

function addColorFieldPicker(field) {
  var template = $("#colorTemplate").clone();

  template.attr("id", "form-group-" + field.name);
  template.attr("data-field-type", field.type);
  template.addClass(classSection); // foldable sections
  if(firstSection) template.addClass("in");

  var id = "input-" + field.name;

  var input = template.find(".minicolors");
  input.attr("id", id);

  if(!field.value.startsWith("rgb("))
    field.value = "rgb(" + field.value;

  if(!field.value.endsWith(")"))
    field.value += ")";

  input.val(field.value);

  var components = rgbToComponents(field.value);

  var redInput = template.find(".color-red-input");
  var greenInput = template.find(".color-green-input");
  var blueInput = template.find(".color-blue-input");

  var redSlider = template.find(".color-red-slider");
  var greenSlider = template.find(".color-green-slider");
  var blueSlider = template.find(".color-blue-slider");

  redInput.attr("id", id + "-red");
  greenInput.attr("id", id + "-green");
  blueInput.attr("id", id + "-blue");

  redSlider.attr("id", id + "-red-slider");
  greenSlider.attr("id", id + "-green-slider");
  blueSlider.attr("id", id + "-blue-slider");

  redInput.val(components.r);
  greenInput.val(components.g);
  blueInput.val(components.b);

  redSlider.val(components.r);
  greenSlider.val(components.g);
  blueSlider.val(components.b);

  redInput.on("change", function() {
    var value = $("#" + id).val();
    var r = $(this).val();
    var components = rgbToComponents(value);
    field.value = r + "," + components.g + "," + components.b;
    $("#" + id).minicolors("value", "rgb(" + field.value + ")");
    redSlider.val(r);
  });

  greenInput.on("change", function() {
    var value = $("#" + id).val();
    var g = $(this).val();
    var components = rgbToComponents(value);
    field.value = components.r + "," + g + "," + components.b;
    $("#" + id).minicolors("value", "rgb(" + field.value + ")");
    greenSlider.val(g);
  });

  blueInput.on("change", function() {
    var value = $("#" + id).val();
    var b = $(this).val();
    var components = rgbToComponents(value);
    field.value = components.r + "," + components.g + "," + b;
    $("#" + id).minicolors("value", "rgb(" + field.value + ")");
    blueSlider.val(b);
  });

  redSlider.on("change", function() {
    var value = $("#" + id).val();
    var r = $(this).val();
    var components = rgbToComponents(value);
    field.value = r + "," + components.g + "," + components.b;
    $("#" + id).minicolors("value", "rgb(" + field.value + ")");
    redInput.val(r);
  });

  greenSlider.on("change", function() {
    var value = $("#" + id).val();
    var g = $(this).val();
    var components = rgbToComponents(value);
    field.value = components.r + "," + g + "," + components.b;
    $("#" + id).minicolors("value", "rgb(" + field.value + ")");
    greenInput.val(g);
  });

  blueSlider.on("change", function() {
    var value = $("#" + id).val();
    var b = $(this).val();
    var components = rgbToComponents(value);
    field.value = components.r + "," + components.g + "," + b;
    $("#" + id).minicolors("value", "rgb(" + field.value + ")");
    blueInput.val(b);
  });

  redSlider.on("change mousemove", function() {
    redInput.val($(this).val());
  });

  greenSlider.on("change mousemove", function() {
    greenInput.val($(this).val());
  });

  blueSlider.on("change mousemove", function() {
    blueInput.val($(this).val());
  });

  input.on("change", function() {
    if (ignoreColorChange) return;

    var value = $(this).val();
    var components = rgbToComponents(value);

    redInput.val(components.r);
    greenInput.val(components.g);
    blueInput.val(components.b);

    redSlider.val(components.r);
    greenSlider.val(components.g);
    blueSlider.val(components.b);

    field.value = components.r + "," + components.g + "," + components.b;
    delayPostColor(field.name, components);
  });

  $("#form").append(template);
}

function addColorFieldPalette(field) {
  var template = $("#colorPaletteTemplate").clone();

  template.addClass(classSection); // foldable sections
  if(firstSection) template.addClass("in");

  var buttons = template.find(".btn-color");

  var label = template.find(".control-label");
  label.text(field.label);

  buttons.each(function(index, button) {
    $(button).click(function() {
      var rgb = $(this).css('backgroundColor');
      var components = rgbToComponents(rgb);

      field.value = components.r + "," + components.g + "," + components.b;
      postColor(field.name, components);

      ignoreColorChange = true;
      var id = "#input-" + field.name;
      $(id).minicolors("value", "rgb(" + field.value + ")");
      $(id + "-red").val(components.r);
      $(id + "-green").val(components.g);
      $(id + "-blue").val(components.b);
      $(id + "-red-slider").val(components.r);
      $(id + "-green-slider").val(components.g);
      $(id + "-blue-slider").val(components.b);
      ignoreColorChange = false;
    });
  });

  $("#form").append(template);
}

function addSectionField(field) {
  var template = $("#sectionTemplate").clone();
  firstSection = classSection=="default"?true:false;
  classSection = field.name;
  template.attr("id", "form-group-section-" + field.name);
  template.attr("data-field-type", field.type);
  
  var label = template.find(".my-control-label");
  label.attr("for", "input-" + field.name);
  label.attr("data-target", "."+classSection);
  label.text(field.label);
  

  $("#form").append(template);
}

function updateFieldValue(name, value) {
  var group = $("#form-group-" + name);

  var type = group.attr("data-field-type");

  if (type == "0") { // NumberFieldType  : 0,
    var input = group.find(".form-control");
    input.val(value);
  } else if (type == "1") { // BooleanFieldType : 1
    var btnOn = group.find("#btnOn" + name);
    var btnOff = group.find("#btnOff" + name);

    btnOn.attr("class", value ? "btn btn-primary" : "btn btn-default");
    btnOff.attr("class", !value ? "btn btn-primary" : "btn btn-default");

  } else if (type == "2") { // SelectFieldType  : 2
    var select = group.find(".form-control");
    select.val(value);
  } else if (type == "3") { // ColorFieldType   : 3
    var input = group.find(".form-control");
    input.val("rgb(" + value + ")");
  }
};

function setBooleanFieldValue(field, btnOn, btnOff, value) {
  field.value = value;

  btnOn.attr("class", field.value ? "btn btn-primary" : "btn btn-default");
  btnOff.attr("class", !field.value ? "btn btn-primary" : "btn btn-default");

  postValue(field.name, field.value);
}

function postValue(name, value) {
  updateStatus("Set " + name + ": " + value + ", please wait...");

  var body = { name: name, value: value };

  $.post(urlBase + "/set?" + name + "=" + value, body, function(data) {
    if (data.name != null) {
      updateStatus("Set /set?" + name + ": " + value);
    } else {
      updateStatus("Set /set?" + name + ": " + value);
    }
  });
  updateStatus("Done...");
  if(ws_connected)
  {
    ws.send("{\"" + name + "\": " + value + "}");
  }
}

function delayPostValue(name, value) {
  clearTimeout(postValueTimer);
  postValueTimer = setTimeout(function() {
    postValue(name, value);
  }, 300);
}

function postColor(name, value) {
  updateStatus("Set " + name + ": " + value.r + "," + value.g + "," + value.b + ", please wait...");

  var body = { name: name, r: value.r, g: value.g, b: value.b };

  $.post(urlBase + "/set?" + name + "=" + name + "&r=" + value.r + "&g=" + value.g + "&b=" + value.b, body, function(data) {
    updateStatus("Set /set?" + name + "=" + name + "&r=" + value.r + "&g=" + value.g + "&b=" + value.b);
  })
  .fail(function(textStatus, errorThrown) { updateStatus("Error: " + textStatus + " " + errorThrown); });
  
  if(ws_connected)
  {
    ws.send("{\"" + name + "\": " + value + "}");
  }
}

function delayPostColor(name, value) {
  clearTimeout(postColorTimer);
  postColorTimer = setTimeout(function() {
    postColor(name, value);
  }, 300);
}

function componentToHex(c) {
  var hex = c.toString(16);
  return hex.length == 1 ? "0" + hex : hex;
}

function rgbToHex(r, g, b) {
  return "#" + componentToHex(r) + componentToHex(g) + componentToHex(b);
}

function rgbToComponents(rgb) {
  var components = {};

  rgb = rgb.match(/^rgb\((\d+),\s*(\d+),\s*(\d+)\)$/);
  components.r = parseInt(rgb[1]);
  components.g = parseInt(rgb[2]);
  components.b = parseInt(rgb[3]);

  return components;
}