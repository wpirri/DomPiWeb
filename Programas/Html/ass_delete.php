<?php
$TITLE='Borrar Objeto'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<div id='ass_delete_back_btn' class='back-btn' onclick="window.location.replace('ass_list.php');" >
	<img id='ass_delete_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='ass_delete_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='ass_delete_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Borrar
</div>

<div id='ass_delete_div' class='abm-div'></div>

<script type="text/javascript" >
	function fillAssDelete(json_list, dst_div, title) { 
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
			if(headers[i] == 'Id') {
				output += '&nbsp;';
			} else if(headers[i] == 'Tipo') {
				output += TipoAss[val].label;
			} else {
				output += val;
			}
			output += '</td>';
			output += '</tr>\n';
		}
		output += '</table>\n';
		document.getElementById(dst_div).innerHTML = output;
	} 

    function LoadData(msg) {
        fillAssDelete(JSON.parse(msg).response, 'ass_delete_div', '<?php echo $TITLE; ?>');
    }

    function LoadHWData(msg) {
        loadHWTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=delete&Id=<?php echo $_GET['Id']; ?>', null, false);
        window.location.replace('ass_list.php');
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmhw.cgi', LoadHWData, false);
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
