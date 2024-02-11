<?php
$TITLE='Particion de Alarma Nueva'; 
include('head-abm.php');
?>

<body  onload="OnLoad();">

<form id="add_form" name="add_form" method="post">

<div id='alarm_add_back_btn' class='back-btn' onclick="window.location.replace('alarm_list.php');" >
	<img id='alarm_add_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='alarm_add_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='alarm_add_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='alarm_add_div' class='abm-div'></div>

<script type="text/javascript" >
	function fillAlarmForm(json_list, dst_div, title) {
		// Getting the all column names 
		var headers = getAbmTableHedaer(json_list);
		var output = '<p class=abm-table-title>' + title + '</p>\n<table class=abm-table id=abm_edit_table>\n';
		var i = 0;

		// Header
		for (i = 0; i < headers.length; i++) { 
			output += '<tr>';
			output += '<th>';
			if(headers[i] == 'Id') { output += '&nbsp;'; }
			else if(headers[i] == 'Particion') { output += '&nbsp;'; }
			else { output += headers[i]; }
			output += '</th>';
			output += '<td>';
			if(headers[i] == 'Id') {
				output += '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" />';
			} else if(headers[i] == 'Particion') {
				output += '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" />';
			} else if(headers[i] == 'Entrada_Act_Total') {
				output += fillSimpleList(headers[i], TablaAlarmaE,0);
			} else if(headers[i] == 'Entrada_Act_Parcial') {
				output += fillSimpleList(headers[i], TablaAlarmaE,0);
			} else if(headers[i] == 'Testigo_Activacion') {
				output += fillSimpleList(headers[i], TablaAlarmaS,0);
			} else if(headers[i] == 'Estado_Activacion') {
				output += fillSimpleList(headers[i], TipoActivacionAlarma);
			} else if(headers[i] == 'Objeto_Zona') {
				output += fillSimpleList(headers[i], TablaAlarmaE,0);
			} else if(headers[i] == 'Tipo_Zona') {
				output += fillSimpleList(headers[i], TipoZonaAlarma);
			} else if(headers[i] == 'Objeto_Salida') {
				output += fillSimpleList(headers[i], TablaAlarmaS,0);
			} else if(headers[i] == 'Tipo_Salida') {
				output += fillSimpleList(headers[i], TipoSalidaAlarma);
			} else {
				output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" />';
			}
			output += '</td>';
			output += '</tr>\n';
		}
		output += '</table>\n';
		document.getElementById(dst_div).innerHTML = output;
	}

    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=get_part&Id=0', LoadData, false);
    }

    function LoadData(msg) {
        fillAlarmForm(JSON.parse(msg).response, 'alarm_add_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=add_part', null, false, collectFormData('add_form'));

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
