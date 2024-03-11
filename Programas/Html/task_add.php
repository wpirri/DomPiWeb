<?php
$TITLE='Tarea Nueva'; 
include('head-abm.php');
?>

<body  onload="OnLoad();">

<form id="add_form" name="add_form" method="post">

<div id='task_add_back_btn' class='back-btn' onclick="window.location.replace('task_list.php');" >
	<img id='task_add_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='task_add_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='task_add_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='task_add_div' class='abm-div'></div>

<script type="text/javascript" >

    function fillTaskForm(json_list, dst_div, title) {
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
            } else if(headers[i] == 'Objeto_Destino') {
                output += fillSimpleList(headers[i], TablaAssOut,0);
            } else if(headers[i] == 'Grupo_Destino') {
                output += fillSimpleList(headers[i], TablaGrupos,0);
            } else if(headers[i] == 'Funcion_Destino') {
                output += fillSimpleList(headers[i], ListaVacia);
            } else if(headers[i] == 'Variable_Destino') {
                output += fillSimpleList(headers[i], ListaVacia);
            } else if(headers[i] == 'Evento') {
                output += fillSimpleList(headers[i], TablaAcciones);
            } else if(headers[i] == 'Mes') {
                output += fillSimpleList(headers[i], TablaMeses);
            } else if(headers[i] == 'Dia') {
                output += fillSimpleList(headers[i], TablaDias);
            } else if(headers[i] == 'Hora') {
                output += fillSimpleList(headers[i], TablaHoras);
            } else if(headers[i] == 'Minuto') {
                output += fillSimpleList(headers[i], TablaMinutos);
            } else if(headers[i] == 'Dias_Semana') {
                output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="Lu,Ma,Mi,Ju,Vi,Sa,Do"/>';
            } else {
                output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" />';
            }
            output += '</td>';
            output += '</tr>\n';
        }
        output += '</table>\n';
        document.getElementById(dst_div).innerHTML = output;
    }

    function LoadData(msg) {
        fillTaskForm(JSON.parse(msg).response, 'task_add_div', '<?php echo $TITLE; ?>');
    }

    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmgroup.cgi', LoadGrpData, false);
    }

    function LoadGrpData(msg) {
        loadGrpTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmat.cgi?funcion=get&Id=0', LoadData, false);
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmat.cgi?funcion=add', null, false, collectFormData('add_form'));

        window.location.replace('task_list.php');
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
