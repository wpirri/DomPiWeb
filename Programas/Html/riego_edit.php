<?php
$TITLE='Editar Circuito de Riego'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<form id="edit_form" name="edit_form" method="post">

<div id='riego_edit_back_btn' class='back-btn' onclick="window.location.replace('riego_list.php');" >
	<img id='riego_edit_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='riego_edit_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='riego_edit_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='riego_edit_div' class='abm-div'></div>

<script type="text/javascript" >
	function fillRiegoEdit(json_list, dst_div, title) { 
		// Getting the all column names 
		var headers = getAbmTableHedaer(json_list);
		var output = '<div class=abm-table-title>&nbsp;' + title + '</div>\n<table class=abm-table id=abm_edit_table>\n';
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
			var val = json_list[0][headers[i]]; 
			if (val == null || val == 'NULL') val = '';   
			output += '<td>';
			if(headers[i] == 'Id') {
				output += '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '"/>';
			} else if(headers[i] == 'Tipo') {
				output += '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '"/>';
			} else if(headers[i] == 'Objeto_Salida') {
				output += fillSimpleList(headers[i], TablaAssOut, val);
			} else if(headers[i] == 'Grupo_Salida') {
				output += fillSimpleList(headers[i], TablaGrupos, val);
			} else if(headers[i] == 'Funcion_Salida') {
				output += fillSimpleList(headers[i], ListaVacia);
			} else if(headers[i] == 'Variable_Salida') {
				output += fillSimpleList(headers[i], ListaVacia);
			} else if(headers[i] == 'Objeto_Sensor') {
				output += fillSimpleList(headers[i], TablaAssIn, val);
			} else if(headers[i] == 'Grupo_Visual') {
				output += fillSimpleList(headers[i], GrupoVisual, val);
			} else if(headers[i] == 'Enviar_Max') {
				output += fillSimpleList(headers[i], TablaAcciones, val);
			} else if(headers[i] == 'Enviar_Min') {
				output += fillSimpleList(headers[i], TablaAcciones, val);
			} else if(headers[i] == 'Habilitado') {
				output += fillSimpleList(headers[i], ListaOnOffAuto, val);
			} else {
				output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '"/>';
			}
			output += '</td>';
			output += '</tr>\n';
		}
		output += '</table>\n';
		document.getElementById(dst_div).innerHTML = output;
	} 

    function LoadData(msg) {
        fillRiegoEdit(JSON.parse(msg).response, 'riego_edit_div', '<?php echo $TITLE; ?>');
    }

    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmgroup.cgi', LoadGrpData, false);
    }

    function LoadGrpData(msg) {
        loadGrpTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=update', null, false, collectFormData('edit_form'));

        window.location.replace('riego_list.php');
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
