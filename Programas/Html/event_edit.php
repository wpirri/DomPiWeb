<?php
$TITLE='Editar Evento'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<form id="edit_form" name="edit_form" method="post">

<div id='event_edit_back_btn' class='back-btn' onclick="window.location.replace('event_list.php');" >
	<img id='event_edit_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='event_edit_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='event_edit_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='event_edit_div' class='abm-div'></div>

<script type="text/javascript" >
	function fillEvEdit(json_list, dst_div, title) { 
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
			if(headers[i] == 'Id') {
				output += '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '"/>';
			} else if(headers[i] == 'Objeto_Origen') {
				output += fillSimpleList(headers[i], TablaAssIn, val);
			} else if(headers[i] == 'Objeto_Destino') {
				output += fillSimpleList(headers[i], TablaAssOut, val);
			} else if(headers[i] == 'Grupo_Destino') {
				output += fillSimpleList(headers[i], TablaGrupos, val);
			} else if(headers[i] == 'Particion_Destino') {
				output += fillSimpleList(headers[i], TablaParticiones, val);
			} else if(headers[i] == 'Funcion_Destino') {
				output += fillSimpleList(headers[i], ListaVacia);
			} else if(headers[i] == 'Variable_Destino') {
				output += fillSimpleList(headers[i], ListaVacia);
			} else if(headers[i] == 'ON_a_OFF') {
				output += fillSimpleList(headers[i], ListaSiNo, val);
			} else if(headers[i] == 'OFF_a_ON') {
				output += fillSimpleList(headers[i], ListaSiNo, val);
			} else if(headers[i] == 'Enviar') {
				output += fillSimpleList(headers[i], TablaAcciones, val);
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
        fillEvEdit(JSON.parse(msg).response, 'event_edit_div', '<?php echo $TITLE; ?>');
    }

    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmgroup.cgi', LoadGrpData, false);
    }

    function LoadGrpData(msg) {
        loadGrpTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=list_part', LoadPartData, false);
    }

    function LoadPartData(msg) {
        loadPartTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmev.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmev.cgi?funcion=update', null, false, collectFormData('edit_form'));

        window.location.replace('event_list.php');
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
