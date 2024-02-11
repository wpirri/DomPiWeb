<?php
$TITLE='Zonas de Alarma'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='alarmz_list_back_btn' class='back-btn' onclick="window.location.replace('alarm_list.php');" >
	<img id='alarmz_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='alarmz_list_add_btn' class='add-btn' onclick="window.location.replace('alarmz_add.php?Particion=<?php echo $_GET['Id']; ?>');" >
	<img id='alarmz_list_add__icon' class='icon-btn' src='images/add.png'>&nbsp;Nuevo
</div>

<div id='alarmz_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
	function fillAlarmZoneList(json_list, dst_div, title, index_label, edit_link, delete_link) { 
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
				val = '<td><a href="' + edit_link + '?' + index_label + '=' + index_value + '"><img src="images/edit.png"></a></td>' 
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
        fillAlarmZoneList(JSON.parse(msg).response, 'alarmz_list_table_div', '<?php echo $TITLE; ?>', 'Id', 'alarmz_edit.php', 'alarmz_delete.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmalarma.cgi?funcion=list_zona&Particion=<?php echo $_GET['Id']; ?>', LoadData, false);
    }
</script>

</body>

<?php
include('foot.php');
?>
