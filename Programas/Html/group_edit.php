<?php
$TITLE='Editar Grupo'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<form id="edit_form" name="edit_form" method="post">

<div id='group_edit_back_btn' class='back-btn' onclick="window.location.replace('group_list.php');" >
	<img id='group_edit_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='group_edit_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='group_edit_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='group_edit_div' class='abm-div'></div>

<script type="text/javascript" >
	function fillGroupEdit(json_list, dst_div, title) { 
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
			} else if(headers[i] == 'Listado_Objetos') {
				output += fillMultiList(headers[i], 10, TablaAssOut, val.split(','));
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
        fillGroupEdit(JSON.parse(msg).response, 'group_edit_div', '<?php echo $TITLE; ?>');
    }

    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmgroup.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmgroup.cgi?funcion=update', null, false, collectFormData('edit_form'));

        window.location.replace('group_list.php');
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
