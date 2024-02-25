var ListaVacia = [];
ListaVacia[0] = { value: 0, label: 'Ninguno' }

var ListaSiNo = [];
ListaSiNo[0] = { value: 0, label: 'No' }
ListaSiNo[1] = { value: 1, label: 'Si' }

var ListaOnOffAuto = [];
ListaOnOffAuto[0] = { value: 0, label: 'Apagado' }
ListaOnOffAuto[1] = { value: 1, label: 'Encendido' }
ListaOnOffAuto[2] = { value: 2, label: 'Automático' }

var TipoHW = [];
TipoHW[0] = { value: 0, label: 'Ninguno' }
TipoHW[1] = { value: 1, label: 'WiFi I/O' }
TipoHW[2] = { value: 2, label: 'RBPi' }
TipoHW[3] = { value: 3, label: 'DSC' }
TipoHW[4] = { value: 4, label: 'Garnet' }
TipoHW[5] = { value: 10, label: 'WiFi IR' }

var EstadoHW = [];
EstadoHW[0] = { value: 0, label: '<img src="images/no.png">' }
EstadoHW[1] = { value: 1, label: '<img src="images/ok.png">' }

var TipoAss = [];
TipoAss[0] = { value: 0, label: 'Salida' }
TipoAss[1] = { value: 1, label: 'Entrada' }
TipoAss[2] = { value: 2, label: 'Entrada Analogica' }
TipoAss[3] = { value: 3, label: 'Salida Alarma' }
TipoAss[4] = { value: 4, label: 'Entrada Alarma' }
TipoAss[5] = { value: 5, label: 'Salida Pulso' }
TipoAss[6] = { value: 6, label: 'Periferico' }
TipoAss[7] = { value: 7, label: 'Salida Infrarroja' }
TipoAss[8] = { value: 8, label: 'Entrada Infrarroja' }

var PortAss = [];
PortAss[0] =  { value: 'IO1',  		label: 'IO1' }
PortAss[1] =  { value: 'IO2',  		label: 'IO2' }
PortAss[2] =  { value: 'IO3',  		label: 'IO3' }
PortAss[3] =  { value: 'IO4',  		label: 'IO4' }
PortAss[4] =  { value: 'IO5',  		label: 'IO5' }
PortAss[5] =  { value: 'IO6',  		label: 'IO6' }
PortAss[6] =  { value: 'IO7',  		label: 'IO7' }
PortAss[7] =  { value: 'IO8',  		label: 'IO8' }
PortAss[8] =  { value: 'OUT1', 		label: 'OUT1' }
PortAss[9] =  { value: 'OUT2', 		label: 'OUT2' }
PortAss[10] = { value: 'OUT3', 		label: 'OUT3' }
PortAss[11] = { value: 'OUT4', 		label: 'OUT4' }
PortAss[12] = { value: 'OUT5', 		label: 'OUT5' }
PortAss[13] = { value: 'OUT6', 		label: 'OUT6' }
PortAss[14] = { value: 'OUT7', 		label: 'OUT7' }
PortAss[15] = { value: 'OUT8', 		label: 'OUT8' }
PortAss[16] = { value: 'TEMP', 		label: 'Sensor Temperatura' }
PortAss[17] = { value: 'HUM', 		label: 'Sensor Humedad' }
PortAss[18] = { value: 'CARD', 		label: 'Lector Tarjetas' }
PortAss[19] = { value: 'OUTIR',		label: 'Salida IR' }
PortAss[20] = { value: 'INIR',		label: 'Entrada IR' }

var GrupoVisual = [];
GrupoVisual[0] = { value: 0, label: 'Ninguno' }
GrupoVisual[1] = { value: 1, label: 'Alarma' }
GrupoVisual[2] = { value: 2, label: 'Iluminación' }
GrupoVisual[3] = { value: 3, label: 'Puertas' }
GrupoVisual[4] = { value: 4, label: 'Climatización' }
GrupoVisual[5] = { value: 5, label: 'Cámaras' }
GrupoVisual[6] = { value: 6, label: 'Riego' }

var TablaAcciones = [];
TablaAcciones[0] = { value: 0, label: 'Ninguno' }
TablaAcciones[1] = { value: 1, label: 'Encender' }
TablaAcciones[2] = { value: 2, label: 'Apagar' }
TablaAcciones[3] = { value: 3, label: 'Cambiar' }
TablaAcciones[4] = { value: 4, label: 'Pulso' }

var TipoAuto = [];
TipoAuto[0] = { value: 0, label: 'Ninguno' }
TipoAuto[1] = { value: 1, label: 'Riego' }
TipoAuto[2] = { value: 2, label: 'Calefaccion' }
TipoAuto[3] = { value: 3, label: 'Aire Acondicionado' }
TipoAuto[4] = { value: 4, label: 'Foto Celula' }
TipoAuto[5] = { value: 5, label: 'Caldera' }
TipoAuto[6] = { value: 6, label: 'Pileta' }

var TipoZonaAlarma = [];
TipoZonaAlarma[0] = { value: 0, label: 'Normal' }
TipoZonaAlarma[1] = { value: 1, label: 'Demorada' }
TipoZonaAlarma[2] = { value: 2, label: 'Incendio' }
TipoZonaAlarma[3] = { value: 3, label: 'Panico' }
TipoZonaAlarma[4] = { value: 4, label: 'Emergencia Médica' }

var TipoActivacionAlarma = [];
TipoActivacionAlarma[0] = { value: 0, label: 'No' }
TipoActivacionAlarma[1] = { value: 1, label: 'Parcial' }
TipoActivacionAlarma[2] = { value: 2, label: 'Total' }

var TipoSalidaAlarma = [];
TipoSalidaAlarma[0] = { value: 0, label: 'Sirena' }
TipoSalidaAlarma[1] = { value: 1, label: 'Buzer' }
TipoSalidaAlarma[2] = { value: 2, label: 'Testigo' }

/* Arma un Drop down listbox */
function fillSimpleList(name, list, selected, onchange) {
	var out = '';
	if (selected == null) {
		selected = (-1);
	}

	if(onchange == null) {
		out += '<select name="' + name + '" id="' + name + '" class="abm-select">\n';
	} else {
		out += '<select name="' + name + '" id="' + name + '" class="abm-select" onchange="' + onchange + '">\n';
	}

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

/* Arma una lista de selección múltiple */
function fillMultiList(name, size, list, selected) {
	if (selected == null) {
		selected = [];
	}
	var out = '<select name="' + name + '" id="' + name + '" size="' + size + '" class="abm-multi-select" multiple>\n';

	for (var i = 1; i < list.length; i++) {
		if(list[i].value > 0) {
			if( selected != null && selected.includes(list[i].value, 0) ) {
				out += '<option selected value="' + list[i].value + '">' + list[i].label + '</option>\n';
			} else {
				out += '<option value="' + list[i].value + '">' + list[i].label + '</option>\n';
			}
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

/* ==== ABM Generico =========================================================== */

function fillAbmList(json_list, dst_div, title, index_label, edit_link, delete_link) { 
	// Getting the all column names 
	var headers = getAbmTableHedaer(json_list);
	var output = '<div class=abm-table-title>&nbsp;' + title + '</div>\n<table class=abm-list-table>\n';
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
	if(edit_link.length > 0)
		output += '<th>Editar</th>';
	if(delete_link.length > 0)
		output += '<th>Borrar</th>';
	output += '</tr>\n';
	// Datos - Salteo el primero de la lista
	for (i = 0; i < json_list.length; i++) {
		if(json_list[i]['Id'] > 0) {
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
			if(edit_link.length > 0) {
				val = '<td><a href="' + edit_link + '?' + index_label + '=' + index_value + '"><img src="images/edit.png"></a></td>' 
				output += val;
			}
			if(edit_link.length > 0) {
				val = '<td><a href="' + delete_link + '?' + index_label + '=' + index_value + '"><img src="images/delete.png"></a></td>' 
				output += val;
			}
			output += '</tr>\n';
		}
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
	var output = '<div class=abm-table-title>&nbsp;' + title + '</div>\n<table class=abm-table id=abm_edit_table>\n';
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
	var output = '<div class=abm-table-title>&nbsp;' + title + '</div>\n<table class=abm-table id=abm_delete_table>\n';
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

var TablaHW = [];
function loadHWTable(json_list) {
	for (var i = 0; i < json_list.length; i++) { 
		var item = [];
		item['value'] = json_list[i].Id;
		item['label'] = json_list[i].Dispositivo;
		TablaHW[i] = item;
	}	
}

var TablaAssIn = [];
var TablaAssOut = [];
var TablaAlarmaE = [];	// Entradas de alarma
var TablaAlarmaS = [];	// Salidas de alarma
function loadAssTable(json_list) {
	var x = 0;
	var y = 0;
	var ae = 0;
	var as = 0;

	for (var i = 0; i < json_list.length; i++) { 
		var item = [];
		item['value'] = json_list[i].Id;
		item['label'] = json_list[i].Objeto;
		if(json_list[i].Id == 0) {
			TablaAssIn[x++] = item;
			TablaAssOut[y++] = item;
			TablaAlarmaE[ae++] = item;
			TablaAlarmaS[as++] = item;
		}
		else
		{
			if(json_list[i].Tipo ==  1  || 
				json_list[i].Tipo == 2 || 
				json_list[i].Tipo == 4 || 
				json_list[i].Tipo == 6 )
			{
				TablaAssIn[x++] = item;
			}
			if(json_list[i].Tipo ==  0 || 
				json_list[i].Tipo == 3 || 
				json_list[i].Tipo == 5  )
			{
				TablaAssOut[y++] = item;
			}
			if(json_list[i].Tipo ==  3)
			{
				TablaAlarmaS[as++] = item;
			}
			if(json_list[i].Tipo ==  4)
			{
				TablaAlarmaE[ae++] = item;
			}
		}
	}
}

var TablaGrupos = [];
function loadGrpTable(json_list) {
	for (var i = 0; i < json_list.length; i++) { 
		var item = [];
		item['value'] = json_list[i].Id;
		item['label'] = json_list[i].Grupo;
		TablaGrupos[i] = item;
	}	
}

var TablaParticiones = [];
function loadPartTable(json_list) {
	for (var i = 0; i < json_list.length; i++) { 
		var item = [];
		item['value'] = json_list[i].Id;
		item['label'] = json_list[i].Nombre;
		TablaParticiones[i] = item;
	}	
}

var TablaIR = [];
function loadIRTable(json_list) {
	for (var i = 0; i < json_list.length; i++) { 
		var item = [];
		item['value'] = json_list[i].Id;
		item['label'] = json_list[i].Nombre;
		TablaIR[i] = item;
	}	
}


/* Recolección de datos de formulario en una estructura JSon */
function collectFormData(form_name) {
	var kvpairs = [];
	var form = document.getElementById(form_name);

	for ( var i = 0; i < form.elements.length; i++ ) {
		var e = form.elements[i];
		if(e.type == "select-multiple")
		{
			var select_data = '';
			for(var j = 0; j < e.length; j++) {
				if(e[j].selected)
				{
					if(select_data.length > 0) {
						select_data += ',';
					}
					select_data +=e[j].value;
				}
			}
			kvpairs.push(encodeURIComponent(e.name) + '=' + encodeURIComponent(select_data));
		}
		else
		{
			kvpairs.push(encodeURIComponent(e.name) + '=' + encodeURIComponent(e.value));
		}
	}
	return kvpairs.join('&');
}
