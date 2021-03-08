function constructTable(json_list, dst_tbl) { 
	// Getting the all column names 
	var cols = constructTableHeaders(json_list, dst_tbl);   
	// Traversing the JSON data 
	for (var i = 0; i < json_list.length; i++) { 
		var row = $('<tr/>');    
		for (var colIndex = 0; colIndex < cols.length; colIndex++) { 
			var val = json_list[i][cols[colIndex]]; 
			// If there is any key, which is matching 
			// with the column name 
			if (val == null) val = "&nbsp;";   
			row.append($('<td/>').html(val)); 
		} 
		// Adding each row to the table 
		$(dst_tbl).append(row); 
	} 
} 

function constructTableHeaders(json_list, dst_tbl) { 
	var columns = []; 
	var header = $('<tr/>'); 
	for (var i = 0; i < json_list.length; i++) { 
		var row = json_list[i]; 
		for (var k in row) { 
			if ($.inArray(k, columns) == -1) { 
				columns.push(k); 
				// Creating the header 
				header.append($('<th/>').html(k)); 
			} 
		} 
	} 
    // Appending the header to the table 
	$(dst_tbl).append(header); 
	return columns; 
}

function getJsonHeaders(json_list) { 
	var headers = []; 
	for (var i = 0; i < json_list.length; i++) { 
		var row = json_list[i]; 
		for (var k in row) { 
			if ($.inArray(k, headers) == -1) { 
				headers.push(k); 
			} 
		} 
	} 
	return headers; 
}

function fillAbmList(json_list, dst_div, index_label, edit_link, delete_link) { 
	// Getting the all column names 
	var headers = getJsonHeaders(json_list);
	var table = '<table class=abm-list-table>\n';
	var i = 0;
	var j = 0;
	var index_value = '';

	// Header
	table += '<tr>';
	for (i = 0; i < headers.length; i++) { 
		table += '<th>';
		table += headers[i];
		table += '</th>';
	}
	// Agrego las columnas de edición y borrado
	table += '<th>Edit</th>';
	table += '<th>Delete</th>';
	table += '</tr>\n';
	// Datos
	for (i = 0; i < json_list.length; i++) { 
		table += '<tr>';
		index_value = '';
		for (j = 0; j < headers.length; j++) { 
			var val = json_list[i][headers[j]]; 
			// If there is any key, which is matching 
			// with the column name 
			if (val == null) val = "&nbsp;";   
			table += '<td>';
			table += val;
			table += '</td>';
			if(index_label == headers[j]) {
				index_value = val;
			}
		} 
		// Agrego los links de edición y borrado
		val = '<td><a href="' + edit_link + '?' + index_label + '=' + index_value + '"><img src="/images/edit.png"></a></td>' 
		table += val;
		val = '<td><a href="' + delete_link + '?' + index_label + '=' + index_value + '"><img src="/images/delete.png"></a></td>' 
		table += val;
		table += '</tr>\n';
	}
	table += '</table>\n';
	document.getElementById(dst_div).innerHTML = table;
} 

function fillAbmDelete(json_list, dst_div) { 
	// Getting the all column names 
	var headers = getJsonHeaders(json_list);
	var table = '<table class=abm-table id=abm_delete_table>\n';
	var i = 0;

	// Header
	for (i = 0; i < headers.length; i++) { 
		table += '<tr>';
		table += '<th>';
		table += headers[i];
		table += '</th>';
		var val = json_list[0][headers[i]]; 
		if (val == null || val == 'NULL') val = '';   
		table += '<td>';
		table += val;
		table += '</th>';
		table += '</tr>\n';
	}
	table += '</table>\n';
	document.getElementById(dst_div).innerHTML = table;
} 

function fillAbmEdit(json_list, dst_div) { 
	// Getting the all column names 
	var headers = getJsonHeaders(json_list);
	var table = '<table class=abm-table id=abm_edit_table>\n';
	var i = 0;

	// Header
	for (i = 0; i < headers.length; i++) { 
		table += '<tr>';
		table += '<th>';
		table += headers[i];
		table += '</th>';
		var val = json_list[0][headers[i]]; 
		if (val == null || val == 'NULL') val = '';   
		table += '<td>';
		table += '<input type="text" id="';
		table += headers[i] + '" name="';
		table += headers[i] + '" ';
		table += 'class="abm-edit-input-text" value="';
		table += val;
		table += '" />';
		table += '</th>';
		table += '</tr>\n';
	}
	table += '</table>\n';
	document.getElementById(dst_div).innerHTML = table;
} 

function fillAbmForm(json_list, dst_div)
{
	// Getting the all column names 
	var headers = getJsonHeaders(json_list);
	var table = '<table class=abm-table id=abm_edit_table>\n';
	var i = 0;

	// Header
	for (i = 0; i < headers.length; i++) { 
		table += '<tr>';
		table += '<th>';
		table += headers[i];
		table += '</th>';
		table += '<td>';
		table += '<input type="text" id="';
		table += headers[i] + '" name="';
		table += headers[i] + '" ';
		table += 'class="abm-edit-input-text" />';
		table += '</th>';
		table += '</tr>\n';
	}
	table += '</table>\n';
	document.getElementById(dst_div).innerHTML = table;
}