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

var DEBUGME = false; //for console logging. 

var ignoreColorChange = false;
var ws_connected = false;

// Dynamic section-based filtering
var currentSectionFilter = 'all';
var availableSections = [];
var sectionFieldMapping = {};
var dynamicTitle = 'LED Control'; // Default title, will be updated from TitleFieldType field

function setCurrentSection(sectionName) {
  currentSectionFilter = sectionName;
  if (DEBUGME) console.log("Setting current section to: " + sectionName);
  
  // Update navigation highlighting
  $('#dynamicNavigation li').removeClass('active');
  $('#dynamicNavigation li[data-section="' + sectionName + '"]').addClass('active');
  
  // Update page title - first section gets just the dynamic title, others get section name
  var sectionTitle = getSectionDisplayName(sectionName);
  if (availableSections.length > 0 && sectionName === availableSections[0].key) {
    // First section: just show the dynamic title
    $('#pageTitle').html(dynamicTitle);
  } else {
    // Other sections: show dynamic title
    $('#pageTitle').html(dynamicTitle);
  }
  
  // Show/hide fields based on current section
  filterFieldsBySection();
}

function getSectionDisplayName(sectionKey) {
  for (var i = 0; i < availableSections.length; i++) {
    if (availableSections[i].key === sectionKey) {
      return availableSections[i].name;
    }
  }
  return sectionKey;
}

function getSectionDescription(sectionKey) {
  // Get the section label directly from the field - no hardcoded descriptions
  for (var i = 0; i < availableSections.length; i++) {
    if (availableSections[i].key === sectionKey) {
      return availableSections[i].name;
    }
  }
  return 'Section';
}

function shouldShowField(fieldName, fieldType) {
  // Never show section headers - they're replaced by navigation
  if (fieldType === fieldtype.SectionFieldType) {
    return false;
  }
  
  // Always show title
  if (fieldType === fieldtype.TitleFieldType) {
    return true;
  }
  
  // Always show fields from the first section
  if (availableSections.length > 0) {
    var firstSectionFields = sectionFieldMapping[availableSections[0].key];
    if (firstSectionFields && firstSectionFields.indexOf(fieldName) !== -1) {
      return true;
    }
  }
  
  // Show fields that belong to the current section (only if not first section)
  if (currentSectionFilter !== availableSections[0].key) {
    var sectionFields = sectionFieldMapping[currentSectionFilter];
    if (sectionFields && sectionFields.indexOf(fieldName) !== -1) {
      return true;
    }
  }
  
  return false;
}

function filterFieldsBySection() {
  // Remove any existing section divider
  $('#currentSectionDivider').remove();
  
  // Add section divider if not on first section
  if (availableSections.length > 0 && currentSectionFilter !== availableSections[0].key) {
    var sectionName = getSectionDisplayName(currentSectionFilter);
    var divider = $('<div id="currentSectionDivider" class="form-group"><div class="col-sm-12"><hr><small>' + sectionName + '</small><hr></div></div>');
    
    // Find the last field from the first section and insert the divider after it
    var firstSectionFields = sectionFieldMapping[availableSections[0].key];
    if (firstSectionFields && firstSectionFields.length > 0) {
      var lastFirstSectionField = firstSectionFields[firstSectionFields.length - 1];
      var lastElement = $('#form-group-' + lastFirstSectionField);
      if (lastElement.length > 0) {
        lastElement.after(divider);
      }
    }
  }
  
  $('#form .form-group').each(function() {
    var fieldName = $(this).attr('id');
    if (fieldName && fieldName.startsWith('form-group-')) {
      var actualFieldName = fieldName.replace('form-group-', '');
      var fieldType = parseInt($(this).attr('data-field-type'));
      
      if (shouldShowField(actualFieldName, fieldType)) {
        $(this).show();
      } else {
        $(this).hide();
      }
    }
  });
}

function buildDynamicNavigation(fields) {
  availableSections = [];
  sectionFieldMapping = {};
  var currentSection = null;
  
  // Parse fields to extract sections, title, and build mapping
  for (var i = 0; i < fields.length; i++) {
    var field = fields[i];
    
    if (field.type === fieldtype.TitleFieldType) {
      // Update dynamic title from backend
      dynamicTitle = field.label;
      // Update page title and navbar brand
      document.title = dynamicTitle;
      $('#nameLink').text(dynamicTitle);
      // Update the page header title with first section (will be updated when section is selected)
    } else if (field.type === fieldtype.SectionFieldType) {
      currentSection = {
        key: field.name,
        name: field.label
      };
      availableSections.push(currentSection);
      sectionFieldMapping[field.name] = [];
    } else if (currentSection && field.type !== fieldtype.TitleFieldType) {
      // Add non-title fields to current section
      sectionFieldMapping[currentSection.key].push(field.name);
    }
  }
  
  // Filter out sections with no fields
  availableSections = availableSections.filter(function(section) {
    return sectionFieldMapping[section.key] && sectionFieldMapping[section.key].length > 0;
  });
  
  if (DEBUGME) {
    console.log("Available sections:", availableSections);
    console.log("Section field mapping:", sectionFieldMapping);
  }
  
  // Build navigation menu - exclude first section (it's always visible)
  var nav = $('#dynamicNavigation');
  nav.empty();
  
  // Add section-based navigation starting from second section
  for (var i = 1; i < availableSections.length; i++) {
    var section = availableSections[i];
    var navItem = $('<li data-section="' + section.key + '"><a href="#" onclick="setCurrentSection(\'' + section.key + '\'); return false;">' + section.name + '</a></li>');
    nav.append(navItem);
  }
}

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
		if(DEBUGME) console.log("Received Client info with from ID " + data.Client + ", Status: " + data.Status);
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

function updateStatus(newStatus, keepMessage = false, timeout = 4000)
{
	$("#status").html(newStatus);
	clearTimeout(statusActiveTimer);
	if(!keepMessage)
	{
		statusActiveTimer = setTimeout(function(timeout) {
			$("#status").html("");
		}, timeout);
	}
}

$(document).ready(function() {
  updateStatus("Connecting, please wait...", true);
	$.get(urlBase + "/all", function(data) {
		updateStatus("Loading, please wait...", true);
		
		// First pass: Build dynamic navigation from sections
		buildDynamicNavigation(data);
		
		// Set first section as active by default
		if (availableSections.length > 0) {
			setCurrentSection(availableSections[0].key);
		} else {
			$('#pageTitle').html('LED Control <small>No sections available</small>');
		}

		// Second pass: Create all form fields
		var currentSection = 'default';
		var isFirstSection = true;
		
		$.each(data, function(index, field) {
			if (field.type == fieldtype.SectionFieldType) {
				// Update section tracking for field creation
				currentSection = field.name;
				isFirstSection = (currentSection === availableSections[0].key);
			} else if (field.type == fieldtype.NumberFieldType) {
				addNumberField(field, currentSection, isFirstSection);
			} else if (field.type == fieldtype.TitleFieldType) {
				// Skip title fields as they're handled in navigation
			} else if (field.type == fieldtype.BooleanFieldType) {
				addBooleanField(field, currentSection, isFirstSection);
			} else if (field.type == fieldtype.SelectFieldType) {
				addSelectField(field, currentSection, isFirstSection);
			} else if (field.type == fieldtype.ColorFieldType) {
				// addColorFieldPalette(field); // removed this to save space on the page. no need currently
				addColorFieldPicker(field, currentSection, isFirstSection);
			}
		});
		
		// Initialize minicolors after all fields are created
		$(".minicolors").minicolors({
			theme: "bootstrap",
			changeDelay: 200,
			control: "brightness",  // changed to sqare one with brightness to the side
			format: "rgb",
			inline: true,
			swatches: ["FF0000", "FF8000", "FFFF00", "00FF00", "00FFFF", "0000FF", "FF00FF", "FFFFFF"] // some colors from the previous list
		});
		
		// Apply initial section filtering after all fields are created
		filterFieldsBySection();
    })
    .fail(function(errorThrown) {
		console.log("error: " + errorThrown);
    })
	.done(function(name, value, test) {
		updateStatus("Structure ready, updating values", true);
		$.get(urlBase + "/allvalues", function(rec) {
			updateStatus("Loading, current values...", true);
			for(i=0; i<rec.values.length; i++) {
				if(DEBUGME) console.log("Name: " + rec.values[i].name + " value " +  rec.values[i].value);
				updateFieldValue( rec.values[i].name,  rec.values[i].value);
      }
    })
		.fail(function(errorThrown) {
			console.log("error: " + errorThrown);
		})
		.done(function(name, value, test) {
			updateStatus("Ready", true);
		});
	});
  // Automatically close the Bootstrap navbar on click (for mobile)
  $('#dynamicNavigation').on('click', 'a', function() {
    var navbarToggle = $('.navbar-toggle:visible');
    var navbarCollapse = $('.navbar-collapse.in');
    if (navbarToggle.length && navbarCollapse.length) {
      navbarToggle.click(); // Triggers the collapse
    }
  });
});

function addNumberField(field, currentSection, isFirstSection) {
  var template = $("#numberTemplate").clone();

  template.attr("id", "form-group-" + field.name);
  template.attr("data-field-type", field.type);
  template.addClass(currentSection || "default"); // foldable sections
  if(isFirstSection) template.addClass("in");

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
  if (field.value != null)
  {
	input.val(field.value);
	slider.val(field.value);
  }

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

function addBooleanField(field, currentSection, isFirstSection) {
  var template = $("#booleanTemplate").clone();

  template.attr("id", "form-group-" + field.name);
  template.attr("data-field-type", field.type);
  template.addClass(currentSection || "default"); // foldable sections
  if(isFirstSection) template.addClass("in");

  var label = template.find(".control-label");
  label.attr("for", "btn-group-" + field.name);
  label.text(field.label);

  var btngroup = template.find(".btn-group");
  btngroup.attr("id", "btn-group-" + field.name);

  var btnOn = template.find("#btnOn");
  var btnOff = template.find("#btnOff");

  btnOn.attr("id", "btnOn" + field.name);
  btnOff.attr("id", "btnOff" + field.name);

  if(field.value != null)
  {
	if(DEBUGME) console.log("\t\tValue: " + field.value);
	btnOn.attr("class", field.value ? "btn btn-primary" : "btn btn-default");
	btnOff.attr("class", !field.value ? "btn btn-primary" : "btn btn-default");
  }						 
  btnOn.click(function() {
    setBooleanFieldValue(field, btnOn, btnOff, 1)
  });
  btnOff.click(function() {
    setBooleanFieldValue(field, btnOn, btnOff, 0)
  });

  $("#form").append(template);
}

function addSelectField(field, currentSection, isFirstSection) {
  var template = $("#selectTemplate").clone();

  template.attr("id", "form-group-" + field.name);
  template.attr("data-field-type", field.type);
  template.addClass(currentSection || "default"); // foldable sections
  if(isFirstSection) template.addClass("in");

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
  if(field.value != null)
  {
	  if(DEBUGME) console.log("\t\tValue: " + field.value);
	  select.val(field.value);
  }
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



function addColorFieldPicker(field, currentSection, isFirstSection) {
  var template = $("#colorTemplate").clone();

  template.attr("id", "form-group-" + field.name);
  template.attr("data-field-type", field.type);
  template.addClass(currentSection || "default"); // foldable sections
  if(isFirstSection) template.addClass("in");

  var id = "input-" + field.name;

  var input = template.find(".minicolors");
  input.attr("id", id);
  if(field.value == null) field.value = 0;
  var value = colToRGB(field.value);
  if(!value.startsWith("rgb("))
    value = "rgb(" + value;

  if(!value.endsWith(")"))
    value += ")";

  if(DEBUGME) console.log("Field " + field);
  if(DEBUGME) console.log("\tName:  " + field.name);
  if(DEBUGME) console.log("\tValue: " + field.value);
  // fixes #42
  $("#" + id).minicolors("value", value);

  var components = rgbToComponents(value);

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
    if(DEBUGME) console.log("redInput.on change");
    var value = $("#" + id).val();
    var r = $(this).val();
    var components = rgbToComponents(value);
    field.value = r + "," + components.g + "," + components.b;
    $("#" + id).minicolors("value", "rgb(" + field.value + ")");
    redSlider.val(r);
  });

  greenInput.on("change", function() {
    if(DEBUGME) console.log("greenInput.on change");
    var value = $("#" + id).val();
    var g = $(this).val();
    var components = rgbToComponents(value);
    field.value = components.r + "," + g + "," + components.b;
    $("#" + id).minicolors("value", "rgb(" + field.value + ")");
    greenSlider.val(g);
  });

  blueInput.on("change", function() {
    if(DEBUGME) console.log("blueInput.on change");
    var value = $("#" + id).val();
    var b = $(this).val();
    var components = rgbToComponents(value);
    field.value = components.r + "," + components.g + "," + b;
    $("#" + id).minicolors("value", "rgb(" + field.value + ")");
    blueSlider.val(b);
  });

  redSlider.on("change", function() {
    if(DEBUGME) console.log("redSlider.on change");
    var value = $("#" + id).val();
    var r = $(this).val();
    var components = rgbToComponents(value);
    field.value = r + "," + components.g + "," + components.b;
    $("#" + id).minicolors("value", "rgb(" + field.value + ")");
    redInput.val(r);
  });

  greenSlider.on("change", function() {
    if(DEBUGME) console.log("greenSlider.on change");
    var value = $("#" + id).val();
    var g = $(this).val();
    var components = rgbToComponents(value);
    field.value = components.r + "," + g + "," + components.b;
    $("#" + id).minicolors("value", "rgb(" + field.value + ")");
    greenInput.val(g);
  });

  blueSlider.on("change", function() {
    if(DEBUGME) console.log("blueSlider.on change");
    var value = $("#" + id).val();
    var b = $(this).val();
    var components = rgbToComponents(value);
    field.value = components.r + "," + components.g + "," + b;
    $("#" + id).minicolors("value", "rgb(" + field.value + ")");
    blueInput.val(b);
  });

  redSlider.on("change mousemove", function() {
    if(DEBUGME) console.log("redSlider.on change mousemove");
    redInput.val($(this).val());
  });

  greenSlider.on("change mousemove", function() {
    if(DEBUGME) console.log("greenSlider.on change mousemove");
    greenInput.val($(this).val());
  });

  blueSlider.on("change mousemove", function() {
    if(DEBUGME) console.log("blueSlider.on change mousemove");
    blueInput.val($(this).val());
  });

  input.on("change", function() {
    if (ignoreColorChange) return;
    if(DEBUGME) console.log("ColorPicker input.on change");
    var value = $(this).val();
    if(DEBUGME) console.log("ColorPicker input.on value = " + value);
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

function addColorFieldPalette(field, currentSection, isFirstSection) {
  var template = $("#colorPaletteTemplate").clone();

  template.addClass(currentSection || "default"); // foldable sections
  if(isFirstSection) template.addClass("in");

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
  if(DEBUGME) console.log("Updating " + name  + " with " + value);
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
    ignoreColorChange = true;
    var input = group.find(".form-control");
    if(DEBUGME) console.log("Updating " + input.toString());
    if(DEBUGME) console.log("\tName:  " + name);
    if(DEBUGME) console.log("\tValue: " + value);
    var comp = colToCompomponents(value);
    // fixes #42
    $("#" + input["0"].id).minicolors("value", colToRGB(value));
    input["1"].value = comp.r;
    input["2"].value = comp.r;
    input["3"].value = comp.g;
    input["4"].value = comp.g;
    input["5"].value = comp.b;
    input["6"].value = comp.b;
    ignoreColorChange = false;
  }
};

function setBooleanFieldValue(field, btnOn, btnOff, value) {
  field.value = value;

  btnOn.attr("class", field.value ? "btn btn-primary" : "btn btn-default");
  btnOff.attr("class", !field.value ? "btn btn-primary" : "btn btn-default");

  postValue(field.name, field.value);
}

function postValue(name, value) {
  updateStatus("Set " + name + ": " + value + ", please wait...", true);

  var body = { name: name, value: value };

  $.get(urlBase + "/set?" + name + "=" + value, body, function(data) {
    if (data.name != null) {
      updateStatus("Set /set?" + name + ": " + value, true);
    } else {
      updateStatus("Set /set?" + name + ": " + value, true);
    }
  })
  .fail(function(jqXHR, textStatus, error) 
  { 
	updateStatus("Error sending the value!", true);
    if(DEBUGME) console.log("Error: " + error + " txt " + textStatus + " jqXHR " + jqXHR);
  })
  .done(function(name, value, test)
  {
	  updateStatus("success setting " + JSON.stringify(name.currentState), false, 3000);
  });
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
  updateStatus("Set " + name + ": " + value.r + "," + value.g + "," + value.b + ", please wait...", true);

  var body = { name: name, r: value.r, g: value.g, b: value.b };

  $.get(urlBase + "/set?" + name + "=" + name + "&r=" + value.r + "&g=" + value.g + "&b=" + value.b, body, function(data) {
    if(DEBUGME) console.log("Sending Color as = " + "/set?" + name + "=" + name + "&r=" + value.r + "&g=" + value.g + "&b=" + value.b);
    updateStatus("Set /set?" + name + "=" + name + "&r=" + value.r + "&g=" + value.g + "&b=" + value.b, true);
  })
  .fail(function(jqXHR, textStatus, errorThrown) { 
    updateStatus("Error sending the Color!", true);
  })
  .done(function(name, value, test)
  {
	  updateStatus("success setting "  + JSON.stringify(name.currentState), false, 3000);
  });
  
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

function colToCompomponents(col) {
  var comp = {};
  comp.r   = (col>>16)&0xff;
  comp.g   = (col>> 8)&0xff;
  comp.b   = (col)    &0xff;
  return comp;
}

function colToRGB(col)
{
  var r = (col>>16)&0xff;
  var g = (col>> 8)&0xff;
  var b = (col)    &0xff;
  return "rgb("+r+","+g+","+b+")";
}