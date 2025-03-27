<?php
$TITLE='Modificar Objeto'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<form id="edit_form" name="edit_form" method="post">

<div id='ass_edit_back_btn' class='back-btn' onclick="window.location.replace('planta_edit.php');" >
	<img id='ass_edit_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='ass_edit_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='ass_edit_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='ass_edit_div' class='abm-div'></div>

<script type="text/javascript" >
    function fillAssEdit(json_list, dst_div, title) { 
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
                output += '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '" />';
            } else if(headers[i] == 'Dispositivo') {
                output += fillSimpleList(headers[i], TablaHW, val);
            } else if(headers[i] == 'Port') {
                output += fillSimpleList(headers[i], PortAss, val);
            } else if(headers[i] == 'Tipo') {
                output += fillSimpleList(headers[i], TipoAss, val);
            } else if(headers[i] == 'Icono_Apagado') {
                output += fillSimpleList(headers[i], ImagenObjeto, val);
            } else if(headers[i] == 'Icono_Encendido') {
                output += fillSimpleList(headers[i], ImagenObjeto, val);
            } else if(headers[i] == 'Grupo_Visual') {
                output += fillSimpleList(headers[i], GrupoVisual, val);
            } else {
                output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '" />';
            }
            output += '</td>';
            output += '</tr>\n';
        }
        output += '</table>\n';
        document.getElementById(dst_div).innerHTML = output;
    } 

    function LoadData(msg) {
        fillAssEdit(JSON.parse(msg).response, 'ass_edit_div', '<?php echo $TITLE; ?>');
    }

    function LoadHWData(msg) {
        loadHWTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmassign.cgi?funcion=update', null, false, collectFormData('edit_form'));

        setTimeout( "window.location.replace('planta_edit.php')", 1000);
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
