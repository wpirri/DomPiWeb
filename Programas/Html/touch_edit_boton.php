<?php
$TITLE='Editar Boton'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<form id="edit_form" name="edit_form" method="post">

<div id='touch_edit_back_btn' class='back-btn' onclick="window.location.replace('touch_list_boton.php?Id=<?php echo $_GET['Id']; ?>&Pantalla=<?php echo $_GET['Pantalla']; ?>');" >
	<img id='touch_edit_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='touch_edit_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='touch_edit_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='touch_edit_div' class='abm-div'></div>

<script type="text/javascript" >

    function fillTouchEdit(json_list, dst_div, title) { 
        // Getting the all column names 
        var headers = getAbmTableHedaer(json_list);
        var output = '<div class=abm-table-title>&nbsp;' + title + '</div>\n<table class=abm-table id=abm_edit_table>\n';
        var i = 0;

        // Header
        for (i = 0; i < headers.length; i++) {
            output += '<tr>';
            output += '<th>';
            if(headers[i] == 'Dispositivo') { output += '&nbsp;'; }
            else { output += headers[i]; }
            output += '</th>';
            var val = json_list[0][headers[i]]; 
            if (val == null || val == 'NULL') val = '';   
            output += '<td>';
            if(headers[i] == 'Dispositivo') {
                output += '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '" />';
            } else if(headers[i] == 'Pantalla') {
                output += val + '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '" />';
            } else if(headers[i] == 'Boton') {
                output += val + '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '" />';
			} else if(headers[i] == 'Objeto') {
				output += fillSimpleList(headers[i], TablaAssOut, val);
			} else if(headers[i] == 'Evento') {
				output += fillSimpleList(headers[i], ListaEventoTouch, val);
			} else if(headers[i] == 'Redondo') {
				output += fillSimpleList(headers[i], ListaSiNo, val);
			} else if(headers[i] == 'Orientacion') {
				output += fillSimpleList(headers[i], ListaHoVe, val);
            } else {
                output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '" />';
            }
            output += '</td>';
            output += '</tr>\n';
        }
        output += '</table>\n';
        document.getElementById(dst_div).innerHTML = output;
    } 

    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmtouch.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>&Pantalla=<?php echo $_GET['Pantalla']; ?>&Boton=<?php echo $_GET['Boton']; ?>', LoadData, false);
    }

    function LoadData(msg) {
        fillTouchEdit(JSON.parse(msg).response, 'touch_edit_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmtouch.cgi?funcion=update', null, false, collectFormData('edit_form'));

        window.location.replace('touch_list_boton.php?Id=<?php echo $_GET['Id']; ?>&Pantalla=<?php echo $_GET['Pantalla']; ?>');
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
