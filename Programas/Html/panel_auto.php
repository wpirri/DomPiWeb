<?php
$TITLE='Tablero de automatismos'; 
include('head-abm.php');
?>

<body onload='OnLoad();'>

<div id='tablero_list_back_btn' class='back-btn' onclick="window.location.replace('<?php echo $MAIN?>');" >
	<img id='tablero_list_back_icon' class='icon-btn' src='images/back.png'>&nbsp;Volver
</div>

<div id='tablero_list_table_div' class='abm-div'></div>

<script type="text/javascript" >
function ChangeControlSelect(id) {
    var val = document.getElementById('id-' + id + '-control').value;
    newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=update', null, false, 'Id=' + id + '&Habilitado=' + val + '&Actualizar=1');
}

function UpdateData(msg) {
    var i;
    var json_list = JSON.parse(msg).response;

    for (i = 1; i < json_list.length; i++) { 
        if(json_list[i].Estado == 0)
            document.getElementById('id-' + json_list[i].Id + '-estado').src = 'images/no.png';
        else
            document.getElementById('id-' + json_list[i].Id + '-estado').src = 'images/ok.png';
    }
}

function LoadData(msg) {
    try {
        var i;
        var json_list = JSON.parse(msg).response;
        var output = '<p class=abm-table-title>&nbsp;<?php echo $TITLE; ?></p>\n';

        output += '<table class=abm-list-table>\n';
        for (i = 1; i < json_list.length; i++) { 
            output += '<tr>';
            output += '<td>';
            output += json_list[i].Nombre;
            output += '</td><td>';
            output += fillSimpleList('id-' + json_list[i].Id + '-control', ListaOnOffAuto, json_list[i].Control, 'ChangeControlSelect(' + json_list[i].Id + ')');
            output += '</td><td>';
            if(json_list[i].Estado == 0)
                output += '<img id="id-' + json_list[i].Id + '-estado" src="images/no.png">';
            else
                output += '<img id="id-' + json_list[i].Id + '-estado" src="images/ok.png">';
            output += '</td>';
            output += '</tr>\n';
        }
        output += '</table>\n';
        document.getElementById('tablero_list_table_div').innerHTML = output;
    } catch (e) { }

    newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=list', UpdateData, true);
}

function OnLoad() {
    newAJAXCommand('/cgi-bin/abmauto.cgi?funcion=list', LoadData, false);
}
</script>

</body>

<?php
include('foot.php');
?>
