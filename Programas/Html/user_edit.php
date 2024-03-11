<?php
$TITLE='Editar Usuario'; 
include('head-abm.php');
?>

<body onload="OnLoad();">

<form id="edit_form" name="edit_form" method="post">

<div id='user_edit_back_btn' class='back-btn' onclick="window.location.replace('user_list.php');" >
	<img id='user_edit_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='user_edit_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='user_edit_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='user_edit_div' class='abm-div'></div>

<script type="text/javascript" >
    var newCardList = [];

    function ChangeInputTarjeta() {
        document.getElementById('div_tarjeta').innerHTML = '<input type="text" id="Tarjeta" name="Tarjeta" class="abm-edit-input-text" />';
    }

    function fillUserEdit(json_list, dst_div, title) { 
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
            } else if(headers[i] == 'Tarjeta') {
                if(val.length) {
                    newCardList[newCardList.length] = { value: val, label: val }
                }
                output += '<div id="div_tarjeta">';
                output += fillSimpleList(headers[i], newCardList, val);
                output += '&nbsp;&nbsp;&nbsp;';
                output += '<a OnClick="ChangeInputTarjeta();">(Ingreso manual)</a>';
                output += '</div>';
			} else if(headers[i] == 'Hora_Inicio') {
				output += fillSimpleList(headers[i], TablaHoras, val);
			} else if(headers[i] == 'Minuto_Inicio') {
				output += fillSimpleList(headers[i], TablaMinutos, val);
			} else if(headers[i] == 'Hora_Fin') {
				output += fillSimpleList(headers[i], TablaHoras, val);
			} else if(headers[i] == 'Minuto_Fin') {
				output += fillSimpleList(headers[i], TablaMinutos, val);
            } else {
                output += '<input type="text" id="' + headers[i] + '" name="' + headers[i] + '" class="abm-edit-input-text" value="' + val + '" />';
            }
            output += '</td>';
            output += '</tr>\n';
        }
        output += '</table>\n';
        document.getElementById(dst_div).innerHTML = output;
    } 

    function OnLoad() {
        newAJAXCommand('/cgi-bin/abmuser.cgi?funcion=new_card', LoadNewCardList, false);
    }

    function LoadNewCardList(msg) {
        try {
            newCardList = JSON.parse(msg).response;
            newCardList[newCardList.length] = { value: '', label: 'Sin Tarjeta' }
        } catch (error) {
            newCardList = [];
            newCardList[0] = { value: '', label: 'Sin Tarjeta' }
        }
        newAJAXCommand('/cgi-bin/abmuser.cgi?funcion=get&Id=<?php echo $_GET['Id']; ?>', LoadData, false);
    }

    function LoadData(msg) {
        fillUserEdit(JSON.parse(msg).response, 'user_edit_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmuser.cgi?funcion=update', null, false, collectFormData('edit_form'));

        window.location.replace('user_list.php');
    }
</script>

</form>

</body>

<?php
include('foot.php');
?>
