<?php
$TITLE='Usuario Nuevo'; 
include('head-abm.php');
?>

<body  onload="OnLoad();">

<form id="add_form" name="add_form" method="post">

<div id='user_add_back_btn' class='back-btn' onclick="window.location.replace('user_list.php');" >
	<img id='user_add_back_icon' class='icon-btn' src='images/no.png'>&nbsp;Cancelar
</div>

<div id='user_add_save_btn' class='submit-btn' onclick="SaveData();" >
	<img id='user_add_save_icon' class='icon-btn' src='images/ok.png'>&nbsp;Guardar
</div>

<div id='user_add_div' class='abm-div'></div>

<script type="text/javascript" >
    var newCardList = [];

    function fillUserForm(json_list, dst_div, title) {
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
            } else if(headers[i] == 'Tarjeta') {

                output += fillSimpleList('Tarjeta', newCardList, '');
                
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
        newAJAXCommand('/cgi-bin/abmuser.cgi?funcion=get&Id=0', LoadData, false);
    }

    function LoadData(msg) {
        fillUserForm(JSON.parse(msg).response, 'user_add_div', '<?php echo $TITLE; ?>');
    }

    function SaveData() {
        newAJAXCommand('/cgi-bin/abmuser.cgi?funcion=add', null, false, collectFormData('add_form'));

        window.location.replace('user_list.php');
    }

</script>

</form>

</body>

<?php
include('foot.php');
?>
