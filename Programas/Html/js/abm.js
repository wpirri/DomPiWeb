var TipoHW=[];
TipoHW[0] = 'Ninguno';
TipoHW[1] = 'WiFi';
TipoHW[2] = 'DSC';
TipoHW[3] = 'Garnet';

var EstadoHW=[];
EstadoHW[0] = '<img src="images/no.png">';
EstadoHW[1] = '<img src="images/ok.png">';


function fillDropDownList(name, list, selected) {
	if (selected == null) {
		selected = (-1);
	}
	var out = '<select name="' + name + '" id="' + name + '">\n';

	for (var i = 0; i < list.length; i++) {
		if(selected == i) {
			out += '<option selected value="' + i + '">' + list[i] + '</option>\n';
		} else {
			out += '<option value="' + i + '">' + list[i] + '</option>\n';
		}
	}
	out += '</select>\n';
	return out;
}

function constructTable(json_list, dst_tbl) { 
	// Getting the all column names 
	var cols = constructTableHeaders(json_list, dst_tbl);   
	// Traversing the JSON data 
	for (var i = 0; i < json_list.length; i++) { 
		var row = $('<tr/>');    
		for (var colIndex = 0; colIndex < cols.length; colIndex++) { 
			var val = json_list[i][cols[colIndex]]; 
			// If there is any key, which is matching 
			// with the column name 
			if (val == null) val = "&nbsp;";   
			row.append($('<td/>').html(val)); 
		} 
		// Adding each row to the table 
		$(dst_tbl).append(row); 
	} 
} 

function constructTableHeaders(json_list, dst_tbl) { 
	var columns = []; 
	var header = $('<tr/>'); 
	for (var i = 0; i < json_list.length; i++) { 
		var row = json_list[i]; 
		for (var k in row) { 
			if ($.inArray(k, columns) == -1) { 
				columns.push(k); 
				// Creating the header 
				header.append($('<th/>').html(k)); 
			} 
		} 
	} 
    // Appending the header to the table 
	$(dst_tbl).append(header); 
	return columns; 
}

function getAbmTableHedaer(json_list) { 
	var headers = []; 
	for (var i = 0; i < json_list.length; i++) { 
		var row = json_list[i]; 
		for (var k in row) { 
			if ($.inArray(k, headers) == -1) { 
				headers.push(k); 
			} 
		} 
	} 
	return headers; 
}

function fillAbmList(json_list, dst_div, title, index_label, edit_link, delete_link) { 
	// Getting the all column names 
	var headers = getAbmTableHedaer(json_list);
	var output = '<p class=abm-table-title>&nbsp;' + title + '</p>\n<table class=abm-list-table>\n';
	var i = 0;
	var j = 0;
	var index_value = '';

	// Header
	output += '<tr>';
	for (i = 0; i < headers.length; i++) { 
		if(headers[i] != 'Id') {
			output += '<th>';
			output += headers[i];
			output += '</th>';
		}
	}
	// Agrego las columnas de edición y borrado
	output += '<th>Editar</th>';
	output += '<th>Borrar</th>';
	output += '</tr>\n';
	// Datos - Salteo el primero de la lista
	for (i = 1; i < json_list.length; i++) { 
		output += '<tr>';
		index_value = '';
		for (j = 0; j < headers.length; j++) { 
			var val = json_list[i][headers[j]]; 
			if(headers[j] == 'Id') {
				index_value = val;
			} else {
				// If there is any key, which is matching 
				// with the column name 
				if (val == null) val = "&nbsp;";   
				output += '<td>';
				output += val;
				output += '</td>';
			}
		} 
		// Agrego los links de edición y borrado
		val = '<td><a href="' + edit_link + '?' + index_label + '=' + index_value + '"><img src="images/edit.png"></a></td>' 
		output += val;
		val = '<td><a href="' + delete_link + '?' + index_label + '=' + index_value + '"><img src="images/delete.png"></a></td>' 
		output += val;
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 

function fillAbmForm(json_list, dst_div, title)
{
	// Getting the all column names 
	var headers = getAbmTableHedaer(json_list);
	var output = '<p class=abm-table-title>' + title + '</p>\n<table class=abm-table id=abm_edit_table>\n';
	var i = 0;

	// Header
	for (i = 0; i < headers.length; i++) { 
		output += '<tr>';
		output += '<th>';
		if(headers[i] == 'Id') { output += '&nbsp;'; }
		else { output += headers[i]; }
		output += '</th>';
		output += '<td>';
		output += '<input type="';
		if(headers[i] == 'Id') { output += 'hidden'; }
		else { output += 'text'; }
		output += '" id="' + headers[i] + '" name="';
		output += headers[i] + '" ';
		output += 'class="abm-edit-input-text" />';
		output += '</td>';
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
}

function fillAbmEdit(json_list, dst_div, title) { 
	// Getting the all column names 
	var headers = getAbmTableHedaer(json_list);
	var output = '<p class=abm-table-title>&nbsp;' + title + '</p>\n<table class=abm-table id=abm_edit_table>\n';
	var i = 0;

	// Header
	for (i = 0; i < headers.length; i++) {
		output += '<tr>';
		output += '<th>';
		if(headers[i] == 'Id') { output += '&nbsp;'; }
		else { output += headers[i]; }
		output += '</th>';
		var val = json_list[0][headers[i]]; 
		if (val == null || val == 'NULL') val = '';   
		output += '<td>';
		output += '<input type="';
		if(headers[i] == 'Id') { output += 'hidden'; }
		else { output += 'text'; }
		output += '" id="' + headers[i] + '" name="';
		output += headers[i] + '" ';
		output += 'class="abm-edit-input-text" value="';
		output += val;
		output += '" />';
		output += '</td>';
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 

function fillAbmDelete(json_list, dst_div, title) { 
	// Getting the all column names 
	var headers = getAbmTableHedaer(json_list);
	var output = '<p class=abm-table-title>&nbsp;' + title + '</p>\n<table class=abm-table id=abm_delete_table>\n';
	var i = 0;

	// Header
	for (i = 0; i < headers.length; i++) { 
		output += '<tr>';
		output += '<th>';
		if(headers[i] == 'Id') { output += '&nbsp;'; }
		else { output += headers[i]; }
		output += '</th>';
		var val = json_list[0][headers[i]]; 
		if (val == null || val == 'NULL') val = '&nbsp';   
		output += '<td>';
		if(headers[i] == 'Id') { output += '&nbsp;'; }
		else { output += val; }
		output += '</td>';
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 

/* ==== Hardware =========================================================== */

function fillHWList(json_list, dst_div, title, index_label, edit_link, delete_link) { 
	// Getting the all column names 
	var headers = getAbmTableHedaer(json_list);
	var output = '<p class=abm-table-title>&nbsp;' + title + '</p>\n<table class=abm-list-table>\n';
	var i = 0;
	var j = 0;
	var index_value = '';

	// Header
	output += '<tr>';
	for (i = 0; i < headers.length; i++) { 
		if(headers[i] != 'Id') {
			output += '<th>';
			output += headers[i];
			output += '</th>';
		}
	}
	// Agrego las columnas de edición y borrado
	output += '<th>Editar</th>';
	output += '<th>Borrar</th>';
	output += '</tr>\n';
	// Datos - Salteo el primero de la lista
	for (i = 1; i < json_list.length; i++) { 
		output += '<tr>';
		index_value = '';
		for (j = 0; j < headers.length; j++) { 
			var val = json_list[i][headers[j]]; 
			if(headers[j] == 'Id') {
				index_value = val;
			} else {
				// If there is any key, which is matching 
				// with the column name 
				if (val == null) val = "&nbsp;";   
				output += '<td>';
				if(headers[j] == 'Tipo') {
					output += TipoHW[val];
				} else if(headers[j] == 'Estado') {
					output += EstadoHW[val];
				} else {
					output += val;
				}
				output += '</td>';
			}
		} 
		// Agrego los links de edición y borrado
		val = '<td><a href="' + edit_link + '?' + index_label + '=' + index_value + '"><img src="images/edit.png"></a></td>' 
		output += val;
		val = '<td><a href="' + delete_link + '?' + index_label + '=' + index_value + '"><img src="images/delete.png"></a></td>' 
		output += val;
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 

function fillHWForm(json_list, dst_div, title)
{
	// Getting the all column names 
	var headers = getAbmTableHedaer(json_list);
	var output = '<p class=abm-table-title>' + title + '</p>\n<table class=abm-table id=abm_edit_table>\n';
	var i = 0;

	// Header
	for (i = 0; i < headers.length; i++) { 
		output += '<tr>';
		output += '<th>';
		if(headers[i] == 'Id') { output += '&nbsp;'; }
		else { output += headers[i]; }
		output += '</th>';
		output += '<td>';
		if(headers[i] == 'Id') {
			output += '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" />';
		} else if(headers[i] == 'Tipo') {
			output += fillDropDownList(headers[i], TipoHW);
		} else {
			output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" />';
		}
		output += '</td>';
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
}

function fillHWEdit(json_list, dst_div, title) { 
	// Getting the all column names 
	var headers = getAbmTableHedaer(json_list);
	var output = '<p class=abm-table-title>&nbsp;' + title + '</p>\n<table class=abm-table id=abm_edit_table>\n';
	var i = 0;

	// Header
	for (i = 0; i < headers.length; i++) {
		output += '<tr>';
		output += '<th>';
		if(headers[i] == 'Id') { output += '&nbsp;'; }
		else { output += headers[i]; }
		output += '</th>';
		var val = json_list[0][headers[i]]; 
		if (val == null || val == 'NULL') val = '';   
		output += '<td>';
		if(headers[i] == 'Id') {
			output += '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '" />';
		} else if(headers[i] == 'Tipo') {
			output += fillDropDownList(headers[i], TipoHW, val);
		} else {
			output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '" />';
		}
		output += '</td>';
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 

function fillHWDelete(json_list, dst_div, title) { 
	// Getting the all column names 
	var headers = getAbmTableHedaer(json_list);
	var output = '<p class=abm-table-title>&nbsp;' + title + '</p>\n<table class=abm-table id=abm_delete_table>\n';
	var i = 0;

	// Header
	for (i = 0; i < headers.length; i++) { 
		output += '<tr>';
		output += '<th>';
		if(headers[i] == 'Id') { output += '&nbsp;'; }
		else { output += headers[i]; }
		output += '</th>';
		var val = json_list[0][headers[i]]; 
		if (val == null || val == 'NULL') val = '&nbsp';   
		output += '<td>';
		if(headers[i] == 'Id') {
			output += '&nbsp;';
		} else if(headers[i] == 'Tipo') {
			output += TipoHW[val];
		} else {
			output += val;
		}
		output += '</td>';
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 

/* ==== Assign ============================================================= */

function fillAddAssignList(json_list, dst_div, title, add_fcn) { 
	// Getting the all column names 
	var headers = getAbmTableHedaer(json_list);
	var output = '<p class=abm-table-title>&nbsp;' + title + '</p>\n<table class=abm-list-table>\n';
	var i = 0;
	var j = 0;
	var index_value = '';

	// Header
	output += '<tr>';
	for (i = 0; i < headers.length; i++) { 
		output += '<th>';
		output += headers[i];
		output += '</th>';
	}
	// Agrego las columnas de edición y borrado
	output += '<th>Agregar</th>';
	output += '</tr>\n';
	// Datos - Salteo el primero de la lista
	for (i = 1; i < json_list.length; i++) { 
		output += '<tr>';
		index_value = '';
		for (j = 0; j < headers.length; j++) { 
			var val = json_list[i][headers[j]]; 
			// If there is any key, which is matching 
			// with the column name 
			if (val == null) val = "&nbsp;";   
			output += '<td>';
			output += val;
			output += '</td>';
			if(headers[j] == 'Id') {
				index_value = val;
			}
		} 
		// Agrego los links de edición y borrado
		val = '<td><img src="images/edit.png" OnClick="' + add_fcn + '(' + index_value + ');"></a></td>' 
		output += val;
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 
