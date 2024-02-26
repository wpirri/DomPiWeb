<?php
$TITLE='Particiones de Alarma'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='alarm_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $CONFIG_MENU?>');" >
	<img id='alarm_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='alarm_list_add_btn' class='add-btn' onclick="window.location.replace('alarm_add.php');" >
	<img id='alarm_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='alarm_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
	function fillAlarmPartList(json_list, dst_div, title, index_label, edit_link, zone_link, out_link, delete_link) { 
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
		output += '<th>Editar</th>';
		output += '<th>Zonas</th>';
		output += '<th>Salidas</th>';
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
				val = '<td><a href="' + zone_link + '?' + index_label + '=' + index_value + '"><img src="images/edit.png"></a></td>' 
				output += val;
				val = '<td><a href="' + out_link + '?' + index_label + '=' + index_value + '"><img src="images/edit.png"></a></td>' 
				output += val;
				val = '<td><a href="' + delete_link + '?' + index_label + '=' + index_value + '"><img src="images/delete.png"></a></td>' 
				output += val;
				output += '</tr>\n';
			}
		}
		output += '</table>\n';
		document.getElementById(dst_div).innerHTML = output;
	} 

    function LoadData(msg) {
        fillAlarmPartList(JSON.parse(msg).response, 'alarm_list_table_div', '<?php echo $TITLE; ?>', 'Id', 'alarm_edit.php', 'alarmz_list.php', 'alarms_list.php', 'alarm_delete.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=list_part', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
