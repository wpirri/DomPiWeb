var ListaVacia = [];
ListaVacia[0] = { value: 0, label: 'Ninguno' }

var ListaSiNo = [];
ListaSiNo[0] = { value: 0, label: 'No' }
ListaSiNo[1] = { value: 1, label: 'Si' }

var TipoHW = [];
TipoHW[0] = { value: 0, label: 'Ninguno' }
TipoHW[1] = { value: 1, label: 'WiFi' }
TipoHW[2] = { value: 2, label: 'DSC' }
TipoHW[3] = { value: 3, label: 'Garnet' }

var EstadoHW = [];
EstadoHW[0] = { value: 0, label: '<img src="images/no.png">' }
EstadoHW[1] = { value: 1, label: '<img src="images/ok.png">' }

var TablaHW = [];

var TipoAss = [];
TipoAss[0] = { value: 0, label: 'Salida' }
TipoAss[1] = { value: 1, label: 'Entrada' }
TipoAss[2] = { value: 2, label: 'Entrada Analogica' }
TipoAss[3] = { value: 3, label: 'Salida Alarma' }
TipoAss[4] = { value: 4, label: 'Entrada Alarma' }
TipoAss[5] = { value: 5, label: 'Salida Pulso' }
TipoAss[6] = { value: 6, label: 'Periferico' }

var PortAss = [];
PortAss[0] =  { value: 'IO1',  label: 'IO1' }
PortAss[1] =  { value: 'IO2',  label: 'IO2' }
PortAss[2] =  { value: 'IO3',  label: 'IO3' }
PortAss[3] =  { value: 'IO4',  label: 'IO4' }
PortAss[4] =  { value: 'IO5',  label: 'IO5' }
PortAss[5] =  { value: 'IO6',  label: 'IO6' }
PortAss[6] =  { value: 'IO7',  label: 'IO7' }
PortAss[7] =  { value: 'IO8',  label: 'IO8' }
PortAss[8] =  { value: 'OUT1', label: 'OUT1' }
PortAss[9] =  { value: 'OUT2', label: 'OUT2' }
PortAss[10] = { value: 'OUT3', label: 'OUT3' }
PortAss[11] = { value: 'OUT4', label: 'OUT4' }
PortAss[12] = { value: 'OUT5', label: 'OUT5' }
PortAss[13] = { value: 'OUT6', label: 'OUT6' }
PortAss[14] = { value: 'OUT7', label: 'OUT7' }
PortAss[15] = { value: 'OUT8', label: 'OUT8' }
PortAss[16] = { value: 'ANA1', label: 'ANA1' }
PortAss[17] = { value: 'ANA2', label: 'ANA2' }
PortAss[18] = { value: 'ANA3', label: 'ANA3' }
PortAss[19] = { value: 'ANA4', label: 'ANA4' }
PortAss[20] = { value: 'ANA5', label: 'ANA5' }
PortAss[21] = { value: 'ANA6', label: 'ANA6' }
PortAss[22] = { value: 'ANA7', label: 'ANA7' }
PortAss[23] = { value: 'ANA8', label: 'ANA8' }

var TablaAssIn = [];
var TablaAssOut = [];

var GrupoVisual = [];
GrupoVisual[0] = { value: 0, label: 'Ninguno' }
GrupoVisual[1] = { value: 1, label: 'Alarma' }
GrupoVisual[2] = { value: 2, label: 'Luces' }
GrupoVisual[3] = { value: 3, label: 'Puertas' }
GrupoVisual[4] = { value: 4, label: 'Climatización' }
GrupoVisual[5] = { value: 5, label: 'Cámaras' }

function fillDropDownList(name, list, selected) {
	if (selected == null) {
		selected = (-1);
	}
	var out = '<select name="' + name + '" id="' + name + '">\n';

	for (var i = 0; i < list.length; i++) {
		if(selected == list[i].value) {
			out += '<option selected value="' + list[i].value + '">' + list[i].label + '</option>\n';
		} else {
			out += '<option value="' + list[i].value + '">' + list[i].label + '</option>\n';
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

function fillAbmForm(json_list, dst_div, title) {
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
					output += TipoHW[val].label;
				} else if(headers[j] == 'Estado') {
					output += EstadoHW[val].label;
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

function fillHWForm(json_list, dst_div, title) {
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
			output += TipoHW[val].label;
		} else {
			output += val;
		}
		output += '</td>';
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 

function loadHWTable(json_list) {
	for (var i = 0; i < json_list.length; i++) { 
		var item = [];
		item['value'] = json_list[i].Id;
		item['label'] = json_list[i].Dispositivo;
		TablaHW[i] = item;
	}	
}

/* ==== Assign ============================================================= */

function fillAssList(json_list, dst_div, title, index_label, edit_link, delete_link) { 
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
					output += TipoAss[val].label;
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

function fillAssForm(json_list, dst_div, title) {
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
		} else if(headers[i] == 'Dispositivo') {
			output += fillDropDownList(headers[i], TablaHW);
		} else if(headers[i] == 'Port') {
			output += fillDropDownList(headers[i], PortAss);
		} else if(headers[i] == 'Tipo') {
			output += fillDropDownList(headers[i], TipoAss);
		} else if(headers[i] == 'Grupo_Visual') {
			output += fillDropDownList(headers[i], GrupoVisual);
		} else {
			output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" />';
		}
		output += '</td>';
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
}

function fillAssEdit(json_list, dst_div, title) { 
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
		} else if(headers[i] == 'Dispositivo') {
			output += fillDropDownList(headers[i], TablaHW, val);
		} else if(headers[i] == 'Port') {
			output += fillDropDownList(headers[i], PortAss, val);
		} else if(headers[i] == 'Tipo') {
			output += fillDropDownList(headers[i], TipoAss, val);
		} else if(headers[i] == 'Grupo_Visual') {
			output += fillDropDownList(headers[i], GrupoVisual, val);
		} else {
			output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '" />';
		}
		output += '</td>';
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 

function fillAssDelete(json_list, dst_div, title) { 
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
			output += TipoAss[val].label;
		} else {
			output += val;
		}
		output += '</td>';
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 

function loadAssTable(json_list) {
	var x = 0;
	var y = 0;
	for (var i = 0; i < json_list.length; i++) { 
		var item = [];
		item['value'] = json_list[i].Id;
		item['label'] = json_list[i].Objeto;
		if(json_list[i].Tipo == 1 || 
			json_list[i].Tipo == 2 || 
			json_list[i].Tipo == 4 || 
			json_list[i].Tipo == 6) {
			TablaAssIn[x++] = item;
		} else {
			TablaAssOut[y++] = item;
		}
	}	
}

/* ==== Evento ============================================================= */
function fillEvList(json_list, dst_div, title, index_label, edit_link, delete_link) { 
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
				if(headers[j] == 'OFF' || headers[j] == 'ON') {
					output += EstadoHW[val].label;
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

function fillEvForm(json_list, dst_div, title) {
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
		} else if(headers[i] == 'Objeto_Origen') {
			output += fillDropDownList(headers[i], TablaAssIn);
		} else if(headers[i] == 'Objeto_Destino') {
			output += fillDropDownList(headers[i], TablaAssOut);
		} else if(headers[i] == 'Grupo_Destino') {
			output += fillDropDownList(headers[i], ListaVacia);
		} else if(headers[i] == 'Funcion_Destino') {
			output += fillDropDownList(headers[i], ListaVacia);
		} else if(headers[i] == 'Variable_Destino') {
			output += fillDropDownList(headers[i], ListaVacia);
		} else if(headers[i] == 'ON_a_OFF') {
			output += fillDropDownList(headers[i], ListaSiNo);
		} else if(headers[i] == 'OFF_a_ON') {
			output += fillDropDownList(headers[i], ListaSiNo);
		} else {
			output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" />';
		}
		output += '</td>';
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
}

function fillEvEdit(json_list, dst_div, title) { 
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
			output += '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '"/>';
		} else if(headers[i] == 'Objeto_Origen') {
			output += fillDropDownList(headers[i], TablaAssIn, val);
		} else if(headers[i] == 'Objeto_Destino') {
			output += fillDropDownList(headers[i], TablaAssOut, val);
		} else if(headers[i] == 'Grupo_Destino') {
			output += fillDropDownList(headers[i], ListaVacia);
		} else if(headers[i] == 'Funcion_Destino') {
			output += fillDropDownList(headers[i], ListaVacia);
		} else if(headers[i] == 'Variable_Destino') {
			output += fillDropDownList(headers[i], ListaVacia);
		} else if(headers[i] == 'ON_a_OFF') {
			output += fillDropDownList(headers[i], ListaSiNo, val);
		} else if(headers[i] == 'OFF_a_ON') {
			output += fillDropDownList(headers[i], ListaSiNo, val);
		} else {
			output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '"/>';
		}
		output += '</td>';
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 

function fillEvDelete(json_list, dst_div, title) { 
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
			output += TipoAss[val].label;
		} else {
			output += val;
		}
		output += '</td>';
		output += '</tr>\n';
	}
	output += '</table>\n';
	document.getElementById(dst_div).innerHTML = output;
} 
