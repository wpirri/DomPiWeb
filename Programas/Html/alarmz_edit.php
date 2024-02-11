<?php
$TITLE='Editar Zona de Alarma'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<form id="edit_form" name="edit_form" method="post">

<div id='alarmz_edit_back_btn' class='back-btn' onclick="window.location.replace('alarm_list.php');" >
	<img id='alarmz_edit_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='alarmz_edit_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='alarmz_edit_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='alarmz_edit_div' class='abm-div'></div>

<script type="text/javascript" >
	function fillAlarmEdit(json_list, dst_div, title) { 
		// Getting the all column names 
		var headers = getAbmTableHedaer(json_list);
		var output = '<div class=abm-table-title>&nbsp;' + title + '</div>\n<table class=abm-table id=abm_edit_table>\n';
		var i = 0;

		// Header
		for (i = 0; i < headers.length; i++) {
			output += '<tr>';
			output += '<th>';
			if(headers[i] == 'Id') { output += '&nbsp;'; }
			else if(headers[i] == 'Particion') { output += '&nbsp;'; }
			else { output += headers[i]; }
			output += '</th>';
			var val = json_list[0][headers[i]]; 
			if (val == null || val == 'NULL') val = '';   
			output += '<td>';
			if(headers[i] == 'Id') {
				output += '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '"/>';
			} else if(headers[i] == 'Particion') {
				output += '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '"/>';
			} else if(headers[i] == 'Entrada_Act_Total') {
				output += fillSimpleList(headers[i], TablaAlarmaE, val);
			} else if(headers[i] == 'Entrada_Act_Parcial') {
				output += fillSimpleList(headers[i], TablaAlarmaE, val);
			} else if(headers[i] == 'Testigo_Activacion') {
				output += fillSimpleList(headers[i], TablaAlarmaS, val);
			} else if(headers[i] == 'Estado_Activacion') {
				output += fillSimpleList(headers[i], TipoActivacionAlarma, val);
			} else if(headers[i] == 'Objeto_Zona') {
				output += fillSimpleList(headers[i], TablaAlarmaE, val);
			} else if(headers[i] == 'Tipo_Zona') {
				output += fillSimpleList(headers[i], TipoZonaAlarma, val);
			} else if(headers[i] == 'Objeto_Salida') {
				output += fillSimpleList(headers[i], TablaAlarmaS, val);
			} else if(headers[i] == 'Tipo_Salida') {
				output += fillSimpleList(headers[i], TipoSalidaAlarma, val);
			} else {
				output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '"/>';
			}
			output += '</td>';
			output += '</tr>\n';
		}
		output += '</table>\n';
		document.getElementById(dst_div).innerHTML = output;
	} 

    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=get_zona&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }

    function LoadData(msg) {
        fillAlarmEdit(JSON.parse(msg).response, 'alarmz_edit_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=update_zona', null, false, collectFormData('edit_form'));

        window.location.replace('alarm_list.php');
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
