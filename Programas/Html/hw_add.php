<?php
$TITLE='Dispositivo Nuevo'; 
include('head-abm.php');
?>

<body  onload="OnLoad();">

<form id="add_form" name="add_form" method="post">

<div id='hw_add_back_btn' class='back-btn' onclick="window.location.replace('hw_list.php');" >
	<img id='hw_add_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='hw_add_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='hw_add_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='hw_add_div' class='abm-div'></div>

<script type="text/javascript" >
    var listaNewHw = [];

    function fillHWForm(json_list, dst_div, title) {
        // Getting the all column names 
        var headers = getAbmTableHedaer(json_list);
        var output = '<p class=abm-table-title>' + title + '</p>\n<table class=abm-table id=abm_edit_table>\n';
        var i = 0;

        // Header
        for (i = 0; i < headers.length; i++) { 
            output += '<tr>';
            output += '<th>';
            if(headers[i] == 'Id') { output += '&nbsp;'; }
            else { output += headers[i]; }
            output += '</th>';
            output += '<td>';
            if(headers[i] == 'Id') {
                output += '<input type="hidden" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" />';
            } else if(headers[i] == 'MAC' && listaNewHw.length > 0) {
                output += fillSimpleList('MAC', listaNewHw);
            } else if(headers[i] == 'Tipo') {
                output += fillSimpleList('Tipo', TipoHW);
            } else {
                output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" />';
            }
            output += '</td>';
            output += '</tr>\n';
        }
        output += '</table>\n';
        document.getElementById(dst_div).innerHTML = output;
    }

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmhw.cgi?funcion=new_hw', LoadNewHWList, false);
    }

    function LoadNewHWList(msg) {
        try {
            listaNewHw = JSON.parse(msg).response;
        } catch (error) {
            listaNewHw = [];
         }
        newAJAXCommand('/cgi-bin/abmhw.cgi?funcion=get&Id=0', LoadData, false);
    }

    function LoadData(msg) {
        fillHWForm(JSON.parse(msg).response, 'hw_add_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmhw.cgi?funcion=add', null, false, collectFormData('add_form'));
        window.location.replace('hw_list.php');
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
