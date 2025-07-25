var ListaVacia = [];
ListaVacia[0] = { value: 0, label: 'Ninguno' }

var ListaSiNo = [];
ListaSiNo[0] = { value: 0, label: 'No' }
ListaSiNo[1] = { value: 1, label: 'Si' }

var ListaHoVe = [];
ListaHoVe[0] = { value: 0, label: 'Horizontal' }
ListaHoVe[1] = { value: 1, label: 'Vertical' }

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
TipoHW[5] = { value: 5, label: 'Touch' }

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

var TablaMeses = [];
TablaMeses[0] = { value: 0, label: 'Todos' }
TablaMeses[1] = { value: 1, label: 'Enero' }
TablaMeses[2] = { value: 2, label: 'Febrero' }
TablaMeses[3] = { value: 3, label: 'Marzo' }
TablaMeses[4] = { value: 4, label: 'Abril' }
TablaMeses[5] = { value: 5, label: 'Mayo' }
TablaMeses[6] = { value: 6, label: 'Junio' }
TablaMeses[7] = { value: 7, label: 'Julio' }
TablaMeses[8] = { value: 8, label: 'Agosto' }
TablaMeses[9] = { value: 9, label: 'Septiembre' }
TablaMeses[10] = { value: 10, label: 'Octubre' }
TablaMeses[11] = { value: 11, label: 'Noviembre' }
TablaMeses[12] = { value: 12, label: 'Diciembre' }

var TablaDias = [];
TablaDias[0] = { value: 0, label: 'Todos' }
TablaDias[1] = { value: 1, label: '1' }
TablaDias[2] = { value: 2, label: '2' }
TablaDias[3] = { value: 3, label: '3' }
TablaDias[4] = { value: 4, label: '4' }
TablaDias[5] = { value: 5, label: '5' }
TablaDias[6] = { value: 6, label: '6' }
TablaDias[7] = { value: 7, label: '7' }
TablaDias[8] = { value: 8, label: '8' }
TablaDias[9] = { value: 9, label: '9' }
TablaDias[10] = { value: 10, label: '10' }
TablaDias[11] = { value: 11, label: '11' }
TablaDias[12] = { value: 12, label: '12' }
TablaDias[13] = { value: 13, label: '13' }
TablaDias[14] = { value: 14, label: '14' }
TablaDias[15] = { value: 15, label: '15' }
TablaDias[16] = { value: 16, label: '16' }
TablaDias[17] = { value: 17, label: '17' }
TablaDias[18] = { value: 18, label: '18' }
TablaDias[19] = { value: 19, label: '19' }
TablaDias[20] = { value: 20, label: '20' }
TablaDias[21] = { value: 21, label: '21' }
TablaDias[22] = { value: 22, label: '22' }
TablaDias[23] = { value: 23, label: '23' }
TablaDias[24] = { value: 24, label: '24' }
TablaDias[25] = { value: 25, label: '25' }
TablaDias[26] = { value: 26, label: '26' }
TablaDias[27] = { value: 27, label: '27' }
TablaDias[28] = { value: 28, label: '28' }
TablaDias[29] = { value: 29, label: '29' }
TablaDias[30] = { value: 30, label: '30' }
TablaDias[31] = { value: 31, label: '31' }

var TablaHoras = [];
TablaHoras[0] = { value: 0, label: '0' }
TablaHoras[1] = { value: 1, label: '1' }
TablaHoras[2] = { value: 2, label: '2' }
TablaHoras[3] = { value: 3, label: '3' }
TablaHoras[4] = { value: 4, label: '4' }
TablaHoras[5] = { value: 5, label: '5' }
TablaHoras[6] = { value: 6, label: '6' }
TablaHoras[7] = { value: 7, label: '7' }
TablaHoras[8] = { value: 8, label: '8' }
TablaHoras[9] = { value: 9, label: '9' }
TablaHoras[10] = { value: 10, label: '10' }
TablaHoras[11] = { value: 11, label: '11' }
TablaHoras[12] = { value: 12, label: '12' }
TablaHoras[13] = { value: 13, label: '13' }
TablaHoras[14] = { value: 14, label: '14' }
TablaHoras[15] = { value: 15, label: '15' }
TablaHoras[16] = { value: 16, label: '16' }
TablaHoras[17] = { value: 17, label: '17' }
TablaHoras[18] = { value: 18, label: '18' }
TablaHoras[19] = { value: 19, label: '19' }
TablaHoras[20] = { value: 20, label: '20' }
TablaHoras[21] = { value: 21, label: '21' }
TablaHoras[22] = { value: 22, label: '22' }
TablaHoras[23] = { value: 23, label: '23' }
TablaHoras[24] = { value: 24, label: 'Todas' }

var TablaMinutos = [];
TablaMinutos[0] = { value: 0, label: '00' }
TablaMinutos[1] = { value: 1, label: '01' }
TablaMinutos[2] = { value: 2, label: '02' }
TablaMinutos[3] = { value: 3, label: '03' }
TablaMinutos[4] = { value: 4, label: '04' }
TablaMinutos[5] = { value: 5, label: '05' }
TablaMinutos[6] = { value: 6, label: '06' }
TablaMinutos[7] = { value: 7, label: '07' }
TablaMinutos[8] = { value: 8, label: '08' }
TablaMinutos[9] = { value: 9, label: '09' }
TablaMinutos[10] = { value: 10, label: '10' }
TablaMinutos[11] = { value: 11, label: '11' }
TablaMinutos[12] = { value: 12, label: '12' }
TablaMinutos[13] = { value: 13, label: '13' }
TablaMinutos[14] = { value: 14, label: '14' }
TablaMinutos[15] = { value: 15, label: '15' }
TablaMinutos[16] = { value: 16, label: '16' }
TablaMinutos[17] = { value: 17, label: '17' }
TablaMinutos[18] = { value: 18, label: '18' }
TablaMinutos[19] = { value: 19, label: '19' }
TablaMinutos[20] = { value: 20, label: '20' }
TablaMinutos[21] = { value: 21, label: '21' }
TablaMinutos[22] = { value: 22, label: '22' }
TablaMinutos[23] = { value: 23, label: '23' }
TablaMinutos[24] = { value: 24, label: '24' }
TablaMinutos[25] = { value: 25, label: '25' }
TablaMinutos[26] = { value: 26, label: '26' }
TablaMinutos[27] = { value: 27, label: '27' }
TablaMinutos[28] = { value: 28, label: '28' }
TablaMinutos[29] = { value: 29, label: '29' }
TablaMinutos[30] = { value: 30, label: '30' }
TablaMinutos[31] = { value: 31, label: '31' }
TablaMinutos[32] = { value: 32, label: '32' }
TablaMinutos[33] = { value: 33, label: '33' }
TablaMinutos[34] = { value: 34, label: '34' }
TablaMinutos[35] = { value: 35, label: '35' }
TablaMinutos[36] = { value: 36, label: '36' }
TablaMinutos[37] = { value: 37, label: '37' }
TablaMinutos[38] = { value: 38, label: '38' }
TablaMinutos[39] = { value: 39, label: '39' }
TablaMinutos[40] = { value: 40, label: '40' }
TablaMinutos[41] = { value: 41, label: '41' }
TablaMinutos[42] = { value: 42, label: '42' }
TablaMinutos[43] = { value: 43, label: '43' }
TablaMinutos[44] = { value: 44, label: '44' }
TablaMinutos[45] = { value: 45, label: '45' }
TablaMinutos[46] = { value: 46, label: '46' }
TablaMinutos[47] = { value: 47, label: '47' }
TablaMinutos[48] = { value: 48, label: '48' }
TablaMinutos[49] = { value: 49, label: '49' }
TablaMinutos[50] = { value: 50, label: '50' }
TablaMinutos[51] = { value: 51, label: '51' }
TablaMinutos[52] = { value: 52, label: '52' }
TablaMinutos[53] = { value: 53, label: '53' }
TablaMinutos[54] = { value: 54, label: '54' }
TablaMinutos[55] = { value: 55, label: '55' }
TablaMinutos[56] = { value: 56, label: '56' }
TablaMinutos[57] = { value: 57, label: '57' }
TablaMinutos[58] = { value: 58, label: '58' }
TablaMinutos[59] = { value: 59, label: '59' }
TablaMinutos[60] = { value: 60, label: 'Todos' }

var ImagenObjeto = [];
ImagenObjeto[0] = { value: 'key.png', label: 'Llave' }
ImagenObjeto[1] = { value: 'lamp1.png', label: 'Luz encendida' }
ImagenObjeto[2] = { value: 'lamp0.png', label: 'Luz apagada' }
ImagenObjeto[3] = { value: 'calef1.png', label: 'Estufa encendida' }
ImagenObjeto[4] = { value: 'calef0.png', label: 'Estufa apagada' }
ImagenObjeto[5] = { value: 'vent1.png', label: 'Extractor encendido' }
ImagenObjeto[6] = { value: 'vent0.png', label: 'Extractor apagado' }
ImagenObjeto[7] = { value: 'aire1.png', label: 'Aire encendido' }
ImagenObjeto[8] = { value: 'aire0.png', label: 'Aire apagado' }
ImagenObjeto[9] = { value: 'campana1.png', label: 'Campanilla encendida' }
ImagenObjeto[10] = { value: 'campana0.png', label: 'Campanilla apagada' }
ImagenObjeto[11] = { value: 'lock1.png', label: 'Candado cerrado' }
ImagenObjeto[12] = { value: 'lock0.png', label: 'Candado abierto' }
ImagenObjeto[13] = { value: 'manguera1.png', label: 'Riego encendido' }
ImagenObjeto[14] = { value: 'manguera0.png', label: 'Riego apagado' }
ImagenObjeto[15] = { value: 'pir1.png', label: 'PIR encendido' }
ImagenObjeto[16] = { value: 'pir0.png', label: 'PIR apagado' }
ImagenObjeto[17] = { value: 'sirena1.png', label: 'Sirena encendida' }
ImagenObjeto[18] = { value: 'sirena0.png', label: 'Sirena apagada' }
ImagenObjeto[19] = { value: 'switch1.png', label: 'Switch encendido' }
ImagenObjeto[20] = { value: 'switch0.png', label: 'Switch apagado' }
ImagenObjeto[21] = { value: 'door1.png', label: 'Puerta abierta' }
ImagenObjeto[22] = { value: 'door0.png', label: 'Puerta cerrada' }
ImagenObjeto[23] = { value: 'window1.png', label: 'Ventana abierta' }
ImagenObjeto[24] = { value: 'window0.png', label: 'Ventana cerrada' }

var ListaEventoTouch = [];
ListaEventoTouch[0] = { value: 0, label: 'Nada' }
ListaEventoTouch[1] = { value: 1, label: 'On' }
ListaEventoTouch[2] = { value: 2, label: 'Off' }
ListaEventoTouch[3] = { value: 3, label: 'Switch' }
ListaEventoTouch[4] = { value: 4, label: 'Pulso' }
ListaEventoTouch[5] = { value: 5, label: '' }
ListaEventoTouch[6] = { value: 6, label: '' }
ListaEventoTouch[7] = { value: 7, label: '' }
ListaEventoTouch[8] = { value: 8, label: '' }
ListaEventoTouch[9] = { value: 9, label: '' }
ListaEventoTouch[10] = { value: 10, label: 'Config' }
ListaEventoTouch[11] = { value: 11, label: 'Home' }
ListaEventoTouch[12] = { value: 12, label: 'Prev' }
ListaEventoTouch[13] = { value: 13, label: 'Next' }

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
