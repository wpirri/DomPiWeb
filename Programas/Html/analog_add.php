<?php
$TITLE='Iluminación Nueva'; 
include('head-abm.php');
?>

<body  onload="OnLoad();">

<form id="add_form" name="add_form" method="post">

<div id='analog_add_back_btn' class='back-btn' onclick="window.location.replace('analog_list.php');" >
	<img id='analog_add_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='analog_add_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='analog_add_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='analog_add_div' class='abm-div'></div>

<script type="text/javascript" >
	function fillAnalogForm(json_list, dst_div, title) {
		// Getting the all column names 
		var headers = getAbmTableHedaer(json_list);
		var output = '<p class=abm-table-title>' + title + '</p>\n<table class=abm-table id=abm_edit_table>\n';
		var i = 0;

		// Header
		for (i = 0; i < headers.length; i++) { 
			output += '<tr>';
			output += '<th>';
			if(headers[i] == 'Id') 
				{ output += '&nbsp;'; }
			else if(headers[i] == 'Tipo') 
				{ output += '&nbsp;'; }
			else 
				{ output += headers[i]; }
			output += '</th>';
			output += '<td>';
			if(headers[i] == 'Id') {
				output += '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" />';
			} else if(headers[i] == 'Tipo') {
				output += '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" />';
			} else if(headers[i] == 'Objeto_Salida') {
				output += fillSimpleList(headers[i], TablaAssOut,0);
			} else if(headers[i] == 'Grupo_Salida') {
				output += fillSimpleList(headers[i], TablaGrupos,0);
			} else if(headers[i] == 'Funcion_Salida') {
				output += fillSimpleList(headers[i], ListaVacia);
			} else if(headers[i] == 'Variable_Salida') {
				output += fillSimpleList(headers[i], ListaVacia);
			} else if(headers[i] == 'Objeto_Sensor') {
				output += fillSimpleList(headers[i], TablaAssIn,0);
			} else if(headers[i] == 'Grupo_Visual') {
				output += fillSimpleList(headers[i], GrupoVisual);
			} else if(headers[i] == 'Dias_Semana') {
				output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="Lu,Ma,Mi,Ju,Vi,Sa,Do"/>';
			} else if(headers[i] == 'Enviar_Max') {
				output += fillSimpleList(headers[i], TablaAcciones);
			} else if(headers[i] == 'Enviar_Min') {
				output += fillSimpleList(headers[i], TablaAcciones);
			} else if(headers[i] == 'Hora_Inicio') {
				output += fillSimpleList(headers[i], TablaHoras);
			} else if(headers[i] == 'Minuto_Inicio') {
				output += fillSimpleList(headers[i], TablaMinutos);
			} else if(headers[i] == 'Hora_Fin') {
				output += fillSimpleList(headers[i], TablaHoras);
			} else if(headers[i] == 'Minuto_Fin') {
				output += fillSimpleList(headers[i], TablaMinutos);
			} else if(headers[i] == 'Habilitado') {
				output += fillSimpleList(headers[i], ListaOnOffAuto);
			} else {
				output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" />';
			}
			output += '</td>';
			output += '</tr>\n';
		}
		output += '</table>\n';
		document.getElementById(dst_div).innerHTML = output;
	}

    function LoadData(msg) {
        fillAnalogForm(JSON.parse(msg).response, 'analog_add_div', '<?php echo $TITLE; ?>');
        document.getElementById('Tipo').value = '4';
    }

    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmgroup.cgi', LoadGrpData, false);
    }

    function LoadGrpData(msg) {
        loadGrpTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=get&Id=0', LoadData, false);
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=add', null, false, collectFormData('add_form'));

        window.location.replace('analog_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmassign.cgi', LoadAssData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
