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
    var TablaMeses = [];
    TablaMeses[0] = { value: 0, label: 'Todos' }
    TablaMeses[1] = { value: 1, label: 'Enero' }
    TablaMeses[2] = { value: 2, label: 'Febrero' }
    TablaMeses[3] = { value: 3, label: 'Marzo' }
    TablaMeses[4] = { value: 4, label: 'Abril' }
    TablaMeses[5] = { value: 5, label: 'Mayo' }
    TablaMeses[6] = { value: 6, label: 'Junio' }
    TablaMeses[7] = { value: 7, label: 'Julio' }
    TablaMeses[8] = { value: 8, label: 'Agosto' }
    TablaMeses[9] = { value: 9, label: 'Septiembre' }
    TablaMeses[10] = { value: 10, label: 'Octubre' }
    TablaMeses[11] = { value: 11, label: 'Noviembre' }
    TablaMeses[12] = { value: 12, label: 'Diciembre' }

    var TablaDias = [];
    TablaDias[0] = { value: 0, label: 'Todos' }
    TablaDias[1] = { value: 1, label: '1' }
    TablaDias[2] = { value: 2, label: '2' }
    TablaDias[3] = { value: 3, label: '3' }
    TablaDias[4] = { value: 4, label: '4' }
    TablaDias[5] = { value: 5, label: '5' }
    TablaDias[6] = { value: 6, label: '6' }
    TablaDias[7] = { value: 7, label: '7' }
    TablaDias[8] = { value: 8, label: '8' }
    TablaDias[9] = { value: 9, label: '9' }
    TablaDias[10] = { value: 10, label: '10' }
    TablaDias[11] = { value: 11, label: '11' }
    TablaDias[12] = { value: 12, label: '12' }
    TablaDias[13] = { value: 13, label: '13' }
    TablaDias[14] = { value: 14, label: '14' }
    TablaDias[15] = { value: 15, label: '15' }
    TablaDias[16] = { value: 16, label: '16' }
    TablaDias[17] = { value: 17, label: '17' }
    TablaDias[18] = { value: 18, label: '18' }
    TablaDias[19] = { value: 19, label: '19' }
    TablaDias[20] = { value: 20, label: '20' }
    TablaDias[21] = { value: 21, label: '21' }
    TablaDias[22] = { value: 22, label: '22' }
    TablaDias[23] = { value: 23, label: '23' }
    TablaDias[24] = { value: 24, label: '24' }
    TablaDias[25] = { value: 25, label: '25' }
    TablaDias[26] = { value: 26, label: '26' }
    TablaDias[27] = { value: 27, label: '27' }
    TablaDias[28] = { value: 28, label: '28' }
    TablaDias[29] = { value: 29, label: '29' }
    TablaDias[30] = { value: 30, label: '30' }
    TablaDias[31] = { value: 31, label: '31' }

    var TablaHoras = [];
    TablaHoras[0] = { value: 0, label: '0' }
    TablaHoras[1] = { value: 1, label: '1' }
    TablaHoras[2] = { value: 2, label: '2' }
    TablaHoras[3] = { value: 3, label: '3' }
    TablaHoras[4] = { value: 4, label: '4' }
    TablaHoras[5] = { value: 5, label: '5' }
    TablaHoras[6] = { value: 6, label: '6' }
    TablaHoras[7] = { value: 7, label: '7' }
    TablaHoras[8] = { value: 8, label: '8' }
    TablaHoras[9] = { value: 9, label: '9' }
    TablaHoras[10] = { value: 10, label: '10' }
    TablaHoras[11] = { value: 11, label: '11' }
    TablaHoras[12] = { value: 12, label: '12' }
    TablaHoras[13] = { value: 13, label: '13' }
    TablaHoras[14] = { value: 14, label: '14' }
    TablaHoras[15] = { value: 15, label: '15' }
    TablaHoras[16] = { value: 16, label: '16' }
    TablaHoras[17] = { value: 17, label: '17' }
    TablaHoras[18] = { value: 18, label: '18' }
    TablaHoras[19] = { value: 19, label: '19' }
    TablaHoras[20] = { value: 20, label: '20' }
    TablaHoras[21] = { value: 21, label: '21' }
    TablaHoras[22] = { value: 22, label: '22' }
    TablaHoras[23] = { value: 23, label: '23' }
    TablaHoras[24] = { value: 24, label: 'Todas' }

    var TablaMinutos = [];
    TablaMinutos[0] = { value: 0, label: '0' }
    TablaMinutos[1] = { value: 1, label: '1' }
    TablaMinutos[2] = { value: 2, label: '2' }
    TablaMinutos[3] = { value: 3, label: '3' }
    TablaMinutos[4] = { value: 4, label: '4' }
    TablaMinutos[5] = { value: 5, label: '5' }
    TablaMinutos[6] = { value: 6, label: '6' }
    TablaMinutos[7] = { value: 7, label: '7' }
    TablaMinutos[8] = { value: 8, label: '8' }
    TablaMinutos[9] = { value: 9, label: '9' }
    TablaMinutos[10] = { value: 10, label: '10' }
    TablaMinutos[11] = { value: 11, label: '11' }
    TablaMinutos[12] = { value: 12, label: '12' }
    TablaMinutos[13] = { value: 13, label: '13' }
    TablaMinutos[14] = { value: 14, label: '14' }
    TablaMinutos[15] = { value: 15, label: '15' }
    TablaMinutos[16] = { value: 16, label: '16' }
    TablaMinutos[17] = { value: 17, label: '17' }
    TablaMinutos[18] = { value: 18, label: '18' }
    TablaMinutos[19] = { value: 19, label: '19' }
    TablaMinutos[20] = { value: 20, label: '20' }
    TablaMinutos[21] = { value: 21, label: '21' }
    TablaMinutos[22] = { value: 22, label: '22' }
    TablaMinutos[23] = { value: 23, label: '23' }
    TablaMinutos[24] = { value: 24, label: '24' }
    TablaMinutos[25] = { value: 25, label: '25' }
    TablaMinutos[26] = { value: 26, label: '26' }
    TablaMinutos[27] = { value: 27, label: '27' }
    TablaMinutos[28] = { value: 28, label: '28' }
    TablaMinutos[29] = { value: 29, label: '29' }
    TablaMinutos[30] = { value: 30, label: '30' }
    TablaMinutos[31] = { value: 31, label: '31' }
    TablaMinutos[32] = { value: 32, label: '32' }
    TablaMinutos[33] = { value: 33, label: '33' }
    TablaMinutos[34] = { value: 34, label: '34' }
    TablaMinutos[35] = { value: 35, label: '35' }
    TablaMinutos[36] = { value: 36, label: '36' }
    TablaMinutos[37] = { value: 37, label: '37' }
    TablaMinutos[38] = { value: 38, label: '38' }
    TablaMinutos[39] = { value: 39, label: '39' }
    TablaMinutos[40] = { value: 40, label: '40' }
    TablaMinutos[41] = { value: 41, label: '41' }
    TablaMinutos[42] = { value: 42, label: '42' }
    TablaMinutos[43] = { value: 43, label: '43' }
    TablaMinutos[44] = { value: 44, label: '44' }
    TablaMinutos[45] = { value: 45, label: '45' }
    TablaMinutos[46] = { value: 46, label: '46' }
    TablaMinutos[47] = { value: 47, label: '47' }
    TablaMinutos[48] = { value: 48, label: '48' }
    TablaMinutos[49] = { value: 49, label: '49' }
    TablaMinutos[50] = { value: 50, label: '50' }
    TablaMinutos[51] = { value: 51, label: '51' }
    TablaMinutos[52] = { value: 52, label: '52' }
    TablaMinutos[53] = { value: 53, label: '53' }
    TablaMinutos[54] = { value: 54, label: '54' }
    TablaMinutos[55] = { value: 55, label: '55' }
    TablaMinutos[56] = { value: 56, label: '56' }
    TablaMinutos[57] = { value: 57, label: '57' }
    TablaMinutos[58] = { value: 58, label: '58' }
    TablaMinutos[59] = { value: 59, label: '59' }
    TablaMinutos[60] = { value: 60, label: 'Todos' }

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
