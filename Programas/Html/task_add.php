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
    function LoadData(msg) {
        fillTaskForm(JSON.parse(msg).response, 'task_add_div', '<?php echo $TITLE; ?>');
    }

    function LoadAssData(msg) {
        loadAssTable(JSON.parse(msg).response);
        newAJAXCommand('/cgi-bin/abmat.cgi?funcion=get&Id=0', LoadData, false);
    }

    function SaveData() {
        /* Send form data to /cgi-bin/abmuser.cgi?funcion=update */

        var kvpairs = [];
        var form = document.getElementById('add_form');

        for ( var i = 0; i < form.elements.length; i++ ) {
            var e = form.elements[i];
            kvpairs.push(encodeURIComponent(e.name) + '=' + encodeURIComponent(e.value));
        }

        newAJAXCommand('/cgi-bin/abmat.cgi?funcion=add', null, false, kvpairs.join('&'));

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