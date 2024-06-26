<?php
// $Id$
function upload_examples()
{
	return drupal_get_form('upload_examples_form');
}
function upload_examples_form($form, &$form_state)
{
	global $user;
	/************************ start approve book details ************************/
	/*$proposal_q = db_query("SELECT * FROM {xcos_textbook_companion_proposal} WHERE uid = %d ORDER BY id DESC LIMIT 1", $user->uid);
	$proposal_data = db_fetch_object($proposal_q);*/
	$query = db_select('textbook_companion_proposal');
	$query->fields('textbook_companion_proposal');
	$query->condition('uid', $user->uid);
	$query->condition('tbc_type','X');
	$query->orderBy('id', 'DESC');
	$query->range(0, 1);
	$result = $query->execute();
	$proposal_data = $result->fetchObject();
	if (!$proposal_data)
	{
		drupal_set_message("Please submit a " . l('proposal', 'proposal') . ".", 'error');
		drupal_goto('');
	} //!$proposal_data
	if ($proposal_data->proposal_status != 1 && $proposal_data->proposal_status != 4)
	{
		switch ($proposal_data->proposal_status)
		{
			case 0:
				drupal_set_message(t('We have already received your proposal. We will get back to you soon.'), 'status');
				drupal_goto('');
				return;
				break;
			case 2:
				drupal_set_message(t('Your proposal has been disapproved. Please create another proposal ' . l('here', 'proposal') . '.'), 'error');
				drupal_goto('');
				return;
				break;
			case 3:
				drupal_set_message(t('Congratulations! You have completed your last book proposal. You have to create another proposal ' . l('here', 'proposal') . '.'), 'status');
				drupal_goto('');
				return;
				break;
			default:
				drupal_set_message(t('Invalid proposal state. Please contact site administrator for further information.'), 'error');
				drupal_goto('');
				return;
				break;
		} //$proposal_data->proposal_status
	} //$proposal_data->proposal_status != 1 && $proposal_data->proposal_status != 4
	/*$preference_q = db_query("SELECT * FROM {xcos_textbook_companion_preference} WHERE proposal_id = %d AND approval_status = 1 LIMIT 1", $proposal_data->id);
	$preference_data = db_fetch_object($preference_q);*/
	$query = db_select('textbook_companion_preference');
	$query->fields('textbook_companion_preference');
	$query->condition('id', $proposal_data->preference_id);
	$query->condition('approval_status', 1);
	$query->range(0, 1);
	$result = $query->execute();
	$preference_data = $result->fetchObject();
	if (!$preference_data)
	{
		drupal_set_message(t('Invalid Book Preference status. Please contact site administrator for further information.'), 'error');
		drupal_goto('');
		return;
	} //!$preference_data
	/************************ end approve book details **************************/
	/* add javascript for automatic book title, check if example uploaded, dependency selection effects */
	if($proposal_data->follow_guidelines == 0){
		drupal_set_message(t('Please read the coding guidelines'), 'error');
		drupal_goto('xcos-textbook-companion/code');
	}
	else{
	$form['#attributes'] = array(
        'enctype' => "multipart/form-data"
    );
$form['book_details']['pref_id'] = array(
    '#type' => 'hidden',
    '#value' => $preference_data->id,    
  );
    $form['book_details']['book'] = array(
        '#type' => 'item',
        '#markup' => $preference_data->book,
        '#title' => t('Title of the Book')
    );
    $form['contributor_name'] = array(
        '#type' => 'item',
        '#markup' => $proposal_data->full_name,
        '#title' => t('Contributor Name')
    );
    $options = array(
        '' => '(Select)'
    );
    /*for ($i = 1; $i <= 100; $i++)
      {
        $options[$i] = $i;
      }*/
    $chapter_number_query = db_query("SELECT id, number from textbook_companion_chapter where preference_id = :preference_id", array(":preference_id" => $preference_data->id));
    while($chapter_number = $chapter_number_query->fetchObject()){
    	$options[$chapter_number->number] = $chapter_number->number;
    	$example_number_query = db_query("SELECT number from textbook_companion_example where chapter_id = :chapter_id", array(":chapter_id" => $chapter_number->id));
    while($example_number = $example_number_query->fetchObject()){
    	$example_number_options[$example_number->number] = $example_number->number;
    }
    }
    $form['number'] = array(
    '#type' => 'select',
    '#title' => t('Chapter No'),
    '#options' => $options,   
    '#multiple' => FALSE,
    '#size' => 1,
    '#required' => TRUE,  
    '#ajax' => array(
        'callback' => 'xcos_ajax_chapter_name_callback',
        ),
    );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Title of the Chapter'),
    '#size' => 40,
    '#maxlength' => 255,
    '#required' => TRUE,    
    '#prefix' => '<div id="ajax-chapter-name-replace">',
    '#suffix' => '</div>',    
  );

  $form['example_number'] = array(
    '#type' => 'select',
    '#title' => t('Example number'),
    '#size' => 1,
    '#options' => $example_number_options,
    '#maxlength' => 255,
    '#required' => TRUE,    
    '#prefix' => '<div id="ajax-example-number-replace">',
    '#suffix' => '</div>',   
    '#ajax' => array(
        'callback' => 'xcos_ajax_example_caption_callback',
        ),
    //'#states'  => 
  );

    /*$form['example_number'] = array(
        '#type' => 'textfield',
        '#title' => t('Example No'),
        '#size' => 5,
        '#maxlength' => 10,
        '#description' => t("Example number should be separated by dots only.<br />Example: 1.1.a &nbsp;or&nbsp; 1.1.1"),
        '#required' => TRUE
    );*/
    $form['example_caption'] = array(
        '#type' => 'textfield',
        '#title' => t('Caption'),
        '#size' => 40,
        '#maxlength' => 255,
        '#prefix' => '<div id="ajax-example-caption-replace">',
    '#suffix' => '</div>',   
        '#description' => t('Example caption should contain only alphabets, numbers and spaces.'),
        '#required' => TRUE
    );
	$form['example_warning'] = array(
		'#type' => 'item',
		'#title' => t('You should upload all the files (main or source files, result files, executable file if any)'),
		'#prefix' => '<div style="color:red">',
		'#suffix' => '</div>'
	);
	$form['sourcefile'] = array(
		'#type' => 'fieldset',
		'#title' => t('Main or Source Files'),
		'#collapsible' => FALSE,
		'#collapsed' => FALSE
	);
	$form['sourcefile']['sourcefile1'] = array(
		'#type' => 'file',
		'#title' => t('Upload main or source file'),
		'#size' => 48,
		'#description' => t('Separate filenames with underscore. No spaces or any special characters allowed in filename.') . '<br />' . t('Allowed file extensions : ') . variable_get('xcos_textbook_companion_source_extensions', '')
	);
	
	/************ END OF EXISTING DEPENDENCIES **************/
	$form['result'] = array(
		'#type' => 'fieldset',
		'#title' => t('Result Files'),
		'#collapsible' => FALSE,
		'#collapsed' => FALSE
	);
	$form['result']['result1'] = array(
		'#type' => 'file',
		'#title' => t('Upload result file'),
		'#size' => 48,
		'#description' => t('Separate filenames with underscore. No spaces or any special characters allowed in filename.') . '<br />' . t('Allowed file extensions : ') . variable_get('xcos_textbook_companion_result_extensions', '')
	);
	$form['result']['result2'] = array(
		'#type' => 'file',
		'#title' => t('Upload result file'),
		'#size' => 48,
		'#description' => t('Separate filenames with underscore. No spaces or any special characters allowed in filename.') . '<br />' . t('Allowed file extensions : ') . variable_get('xcos_textbook_companion_result_extensions', '')
	);
	$form['result']['result3'] = array(
		'#type' => 'file',
		'#title' => t('Upload result file'),
		'#size' => 48,
		'#description' => t('Separate filenames with underscore. No spaces or any special characters allowed in filename.') . '<br />' . t('Allowed file extensions : ') . variable_get('xcos_textbook_companion_result_extensions', '')
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Submit')
	);
	$form['cancel'] = array(
		'#type' => 'markup',
		'#value' => l(t('Cancel'), 'xcos-textbook-companion/code')
	);
	return $form;
}
}
function upload_examples_form_validate($form, &$form_state)
{
	if (!check_name($form_state['values']['name']))
		form_set_error('name', t('Title of the Chapter can contain only alphabets, numbers and spaces.'));
	if (!check_name($form_state['values']['example_caption']))
		form_set_error('example_caption', t('Example Caption can contain only alphabets, numbers and spaces.'));
	if (!check_chapter_number($form_state['values']['example_number']))
		form_set_error('example_number', t('Invalid Example Number. Example Number can contain only alphabets and numbers sepereated by dot.'));
	if (isset($_FILES['files']))
	{
		/* check if atleast one source or result file is uploaded */
		if (!($_FILES['files']['name']['sourcefile1'] || $_FILES['files']['name']['xcos1']))
			form_set_error('sourcefile1', t('Please upload atleast one main or source file or xcos file.'));
		if (!($_FILES['files']['name']['result1']))
			form_set_error('result1', t('Please upload atleast one result file.'));
		/* check for valid filename extensions */
		foreach ($_FILES['files']['name'] as $file_form_name => $file_name)
		{
			if ($file_name)
			{
				/* checking file type */
				if (strstr($file_form_name, 'source'))
					$file_type = 'P';
				else if (strstr($file_form_name, 'result'))
					$file_type = 'Q';
				/*else if (strstr($file_form_name, 'xcos'))
					$file_type = 'X';*/
				else
					$file_type = 'U';
				$allowed_extensions_str = '';
				switch ($file_type)
				{
					case 'P':
						$allowed_extensions_str = variable_get('xcos_textbook_companion_source_extensions', '');
						break;
					case 'Q':
						$allowed_extensions_str = variable_get('xcos_textbook_companion_result_extensions', '');
						break;
					/*case 'X':
						$allowed_extensions_str = variable_get('xcos_textbook_companion_xcos_extensions', '');
						break;*/
				} //$file_type
				$allowed_extensions = explode(',', $allowed_extensions_str);
				$temp_extension = end(explode('.', strtolower($_FILES['files']['name'][$file_form_name])));
				if (!in_array($temp_extension, $allowed_extensions))
					form_set_error($file_form_name, t('Only file with ' . $allowed_extensions_str . ' extensions can be uploaded.'));
				if ($_FILES['files']['size'][$file_form_name] <= 0)
					form_set_error($file_form_name, t('File size cannot be zero.'));
				/* check if valid file name */
				if (!xcos_textbook_companion_check_valid_filename($_FILES['files']['name'][$file_form_name]))
					form_set_error($file_form_name, t('Invalid file name specified. Only alphabets, numbers and underscore is allowed as a valid filename.'));
			} //$file_name
		} //$_FILES['files']['name'] as $file_form_name => $file_name
	} //isset($_FILES['files'])
	/* add javascript again for automatic book title, check if example uploaded, dependency selection effects */
	$chapter_name_js = "(function ($) {
    $('#edit-number').change(function() {
      $.get('" . base_path() . "xcos-textbook-companion/ajax/chapter_title/' + $('#edit-number').val() + '/' + " . $row->pref_id . ", function(data) {
        $('#edit-name').val(data);
      });
    });
    $('#edit-example-number').change(function() {
      $.get('" . base_path() . "xcos-textbook-companion/ajax/example_exists/' + $('#edit-number').val() + '/' + $('#edit-example-number').val(), function(data) {
        if (data) {
           alert(data);
        }
      });
    });
    $('#edit-existing-depfile-dep-book-title').change(function() {
      var dep_selected = ''; 
      /* showing and hiding relevant files */
      $('.form-checkboxes .option').hide();
      $('.form-checkboxes .option').each(function(index) {
        var activeClass = $('#edit-existing-depfile-dep-book-title').val();
        if ($(this).children().hasClass(activeClass)) {
          $(this).show();
        }
        if ($(this).children().attr('checked') == true) {
          dep_selected += $(this).children().next().text() + '<br />';
        }
      });
      /* showing list of already existing dependencies */
      $('#existing_depfile_selected').html(dep_selected);
    });

    $('.form-checkboxes .option').change(function() {
      $('#edit-existing-depfile-dep-book-title').trigger('change');
    });
    $('#edit-existing-depfile-dep-book-title').trigger('change');
   })(jQuery);";
	drupal_add_js($chapter_name_js, 'inline', 'header');
}
function upload_examples_form_submit($form, &$form_state)
{
	global $user;
	$root_path = xcos_textbook_companion_path();
	/************************ start approve book details ************************/
	/*$proposal_q = db_query("SELECT * FROM {xcos_textbook_companion_proposal} WHERE uid = %d ORDER BY id DESC LIMIT 1", $user->uid);
	$proposal_data = db_fetch_object($proposal_q);*/
	$query = db_select('textbook_companion_proposal');
	$query->fields('textbook_companion_proposal');
	$query->condition('uid', $user->uid);
	$query->condition('tbc_type', 'X');
	$query->orderBy('id', 'DESC');
	$query->range(0, 1);
	$result = $query->execute();
	$proposal_data = $result->fetchObject();
	if (!$proposal_data)
	{
		drupal_set_message("Please submit a " . l('proposal', 'proposal') . ".", 'error');
		drupal_goto('');
	} //!$proposal_data
	if ($proposal_data->proposal_status != 1 && $proposal_data->proposal_status != 4)
	{
		switch ($proposal_data->proposal_status)
		{
			case 0:
				drupal_set_message(t('We have already received your proposal. We will get back to you soon.'), 'status');
				drupal_goto('');
				return;
				break;
			case 2:
				drupal_set_message(t('Your proposal has been disapproved. Please create another proposal ' . l('here', 'proposal') . '.'), 'error');
				drupal_goto('');
				return;
				break;
			case 3:
				drupal_set_message(t('Congratulations! You have completed your last book proposal. You have to create another proposal ' . l('here', 'xcos_textbook_companion/proposal') . '.'), 'status');
				drupal_goto('');
				return;
				break;
			default:
				drupal_set_message(t('Invalid proposal state. Please contact site administrator for further information.'), 'error');
				drupal_goto('');
				return;
				break;
		} //$proposal_data->proposal_status
	} //$proposal_data->proposal_status != 1 && $proposal_data->proposal_status != 4
	/*$preference_q = db_query("SELECT * FROM {xcos_textbook_companion_preference} WHERE proposal_id = %d AND approval_status = 1 LIMIT 1", $proposal_data->id);
	$preference_data = db_fetch_object($preference_q);*/
	/*$query = db_select('textbook_companion_preference');
	$query->fields('textbook_companion_preference');
	//$query->condition('proposal_id', $proposal_data->id);
	$query->condition('approval_status', 1);
	$query->range(0, 1);
	$result = $query->execute();
	$preference_data = $result->fetchObject();
	if (!$preference_data)
	{
		drupal_set_message(t('Invalid Book Preference status. Please contact site administrator for further information.'), 'error');
		drupal_goto('');
		return;
	} *///!$preference_data
	/************************ end approve book details **************************/
	$preference_id = $proposal_data->preference_id;
	$dest_path = $preference_id . '/';
	if (!is_dir($root_path . $dest_path))
		mkdir($root_path . $dest_path);
	/* inserting chapter details */
	$chapter_id = 0;
	/*$chapter_result = db_query("SELECT * FROM {xcos_textbook_companion_chapter} WHERE preference_id = %d AND number = %d", $preference_id, $form_state['values']['number']);*/
	$query = db_select('textbook_companion_chapter');
	$query->fields('textbook_companion_chapter');
	$query->condition('preference_id', $preference_id);
	$query->condition('number', $form_state['values']['number']);
	$chapter_result = $query->execute();
	//var_dump($chapter_result->rowCount());die;
	if (!$chapter_row = $chapter_result->fetchObject())
	{
		/*db_query("INSERT INTO {xcos_textbook_companion_chapter} (preference_id, number, name) VALUES (%d, '%s', '%s')",
		$preference_id,
		$form_state['values']['number'],
		$form_state['values']['name']
		);
		$chapter_id = db_last_insert_id('textbook_companion_chapter', 'id'); */
		$query = "INSERT INTO textbook_companion_chapter (preference_id, number, name) VALUES(:preference_id, :number, :name)";
		$args = array(
			":preference_id" => $preference_id,
			":number" => $form_state['values']['number'],
			":name" => $form_state['values']['name']
		);
		$result = db_query($query, $args, array(
			'return' => Database::RETURN_INSERT_ID
		));
		$chapter_id = $result;
	} //!$chapter_row = $chapter_result->fetchObject()
	else
	{
	//var_dump($chapter_row);die;
		$chapter_id = $chapter_row->id;
		/*db_query("UPDATE {xcos_textbook_companion_chapter} SET name = '%s' WHERE id = %d", $form_state['values']['name'], $chapter_id);*/
		$query = db_update('textbook_companion_chapter');
		$query->fields(array(
			'name' => $form_state['values']['name']
		));
		$query->condition('id', $chapter_id);
		$num_updated = $query->execute();
	}
	/*  get example details - dont allow if already example present */
	/*$cur_example_q = db_query("SELECT * FROM {xcos_textbook_companion_example} WHERE chapter_id = %d AND number = '%s'", $chapter_id, $form_state['values']['example_number']);*/
	$query = db_select('textbook_companion_example');
	$query->fields('textbook_companion_example');
	$query->condition('chapter_id', $chapter_id);
	$query->condition('number', $form_state['values']['example_number']);
	$cur_example_q = $query->execute();
	//var_dump($cur_example_q->rowCount());die;
	$cur_example_d = $cur_example_q->fetchObject();
	/*if ($cur_example_d = $cur_example_q->fetchObject())
	{
		//var_dump($cur_example_d);die;
		if ($cur_example_d->xcos_approval_status == 1)
		{
			drupal_set_message(t("Example already approved. Cannot overwrite it."), 'error');
			drupal_goto('xcos-textbook-companion/code');
			return;
		} //$cur_example_d->approval_status == 1
		/*else if ($cur_example_d->xcos_approval_status == 0)
		{
			drupal_set_message(t("Example is under pending review. Delete the example and reupload it."), 'error');
			drupal_goto('xcos-textbook-companion/code');
			return;
		}//$cur_example_d->approval_status == 0
		else
		{
			drupal_set_message(t("Error uploading example. Please contact administrator."), 'error');
			drupal_goto('xcos-textbook-companion/code');
			return;
		}
	}*/ //$cur_example_d = $cur_example_q->fetchObject()
	/* creating directories */
	$dest_path .= 'CH' . $form_state['values']['number'] . '/';
	if (!is_dir($root_path . $dest_path))
		mkdir($root_path . $dest_path);
	$dest_path .= 'EX' . $form_state['values']['example_number'] . '/';
	if (!is_dir($root_path . $dest_path))
		mkdir($root_path . $dest_path);
	/* creating example database entry */
	/*db_query("INSERT INTO {xcos_textbook_companion_example} (chapter_id, number, caption, approval_status, timestamp) VALUES (%d, '%s', '%s', %d, %d)",
	$chapter_id,
	$form_state['values']['example_number'],
	$form_state['values']['example_caption'],
	0,
	time()
	);
	$example_id = db_last_insert_id('textbook_companion_example', 'id');*/
	//var_dump($chapter_id);die;
	//var_dump($chapter_id);die;
	$query = "UPDATE textbook_companion_example set xcos_approver_uid =:xcos_approver_uid , caption = :caption, xcos_approval_status = :xcos_approval_status, timestamp=:timestamp where chapter_id=:chapter_id and number =:number";
	$args = array(
		":xcos_approver_uid" => 0,
		":caption" => $form_state['values']['example_caption'],
		":xcos_approval_status" => 0,
		":timestamp" => time(),
		":chapter_id" => $chapter_id,
		":number" => $form_state['values']['example_number'],
	);
	$result = db_query($query, $args);
	$example_id = $result;
	//var_dump($example_id);die;
	/* linking existing dependencies */
	foreach ($_FILES['files']['name'] as $file_form_name => $file_name)
	{
		if ($file_name)
		{
			/* checking file type */
			if (strstr($file_form_name, 'source'))
				$file_type = 'P';
			else if (strstr($file_form_name, 'result'))
				$file_type = 'Q';
			else
				$file_type = 'U';
			if (file_exists($root_path . $dest_path . $_FILES['files']['name'][$file_form_name]))
			{
				drupal_set_message(t("Error uploading file. File !filename already exists.", array(
					'!filename' => $_FILES['files']['name'][$file_form_name]
				)), 'error');
				return;
			} //file_exists($root_path . $dest_path . $_FILES['files']['name'][$file_form_name])
			/* uploading file */
			if (move_uploaded_file($_FILES['files']['tmp_name'][$file_form_name], $root_path . $dest_path . $_FILES['files']['name'][$file_form_name]))
			{
				/* for uploaded files making an entry in the database */
				/*db_query("INSERT INTO {xcos_textbook_companion_example_files} (example_id, filename, filepath, filemime, filesize, filetype, timestamp)
				VALUES (%d, '%s', '%s', '%s', %d, '%s', %d)",
				$example_id,
				$_FILES['files']['name'][$file_form_name],
				$dest_path . $_FILES['files']['name'][$file_form_name],
				$_FILES['files']['type'][$file_form_name],
				$_FILES['files']['size'][$file_form_name],
				$file_type,
				time()
				);*/
				$query = "INSERT INTO textbook_companion_example_files (example_id, caption, filename, filepath,filemime, filesize, filetype, timestamp)
          VALUES (:example_id, :caption, :filename, :filepath,:filemime, :filesize, :filetype, :timestamp)";
				$args = array(
					":example_id" => $cur_example_d->id,
					":caption" => 'None',
					":filename" => $_FILES['files']['name'][$file_form_name],
					":filepath" => $dest_path . $_FILES['files']['name'][$file_form_name],
					":filemime" => $_FILES['files']['type'][$file_form_name],
					":filesize" => $_FILES['files']['size'][$file_form_name],
					":filetype" => $file_type,
					":timestamp" => time()
				);
				$result = db_query($query, $args, array(
					'return' => Database::RETURN_INSERT_ID
				));
				drupal_set_message($file_name . ' uploaded successfully.', 'status');
			} //move_uploaded_file($_FILES['files']['tmp_name'][$file_form_name], $root_path . $dest_path . $_FILES['files']['name'][$file_form_name])
			else
			{
				drupal_set_message('Error uploading file : ' . $dest_path . '/' . $file_name, 'error');
			}
		} //$file_name
	} //$_FILES['files']['name'] as $file_form_name => $file_name
	drupal_set_message('Example uploaded successfully.', 'status');
	/* sending email */
	$email_to = $user->mail;
	$from = variable_get('xcos_textbook_companion_from_email', '');
	$bcc = variable_get('xcos_textbook_companion_emails', '');
	$cc = variable_get('xcos_textbook_companion_cc_emails', '');
	$params['example_uploaded']['example_id'] = $cur_example_d->id;
	$params['example_uploaded']['user_id'] = $user->uid;
	$params['example_uploaded']['headers'] = array(
		'From' => $from,
		'MIME-Version' => '1.0',
		'Content-Type' => 'text/plain; charset=UTF-8; format=flowed; delsp=yes',
		'Content-Transfer-Encoding' => '8Bit',
		'X-Mailer' => 'Drupal',
		'Cc' => $cc,
		'Bcc' => $bcc
	);
	if (!drupal_mail('xcos_textbook_companion', 'example_uploaded', $email_to, language_default(), $params, $from, TRUE))
		drupal_set_message('Error sending email message.', 'error');
	drupal_goto('xcos-textbook-companion/code');
}
/******************************************************************************/
/***************************** DELETE EXAMPLE *********************************/
/******************************************************************************/
function _upload_examples_delete()
{
	global $user;
	$root_path = xcos_textbook_companion_path();
	$example_id = arg(3);
	/* check example */
	/*$example_q = db_query("SELECT * FROM {xcos_textbook_companion_example} WHERE id = %d LIMIT 1", $example_id);
	$example_data = db_fetch_object($example_q);*/
	$query = db_select('textbook_companion_example');
	$query->fields('textbook_companion_example');
	$query->condition('id', $example_id);
	$query->range(0, 1);
	$result = $query->execute();
	$example_data = $result->fetchObject();
	if (!$example_data)
	{
		drupal_set_message('Invalid example.', 'error');
		drupal_goto('xcos-textbook-companion/code');
		return;
	} //!$example_data
	if ($example_data->approval_status != 0)
	{
		drupal_set_message('You cannnot delete an example after it has been approved. Please contact site administrator if you want to delete this example.', 'error');
		drupal_goto('xcos-textbook-companion/code');
		return;
	} //$example_data->approval_status != 0
	/*$chapter_q = db_query("SELECT * FROM {xcos_textbook_companion_chapter} WHERE id = %d LIMIT 1", $example_data->chapter_id);
	$chapter_data = db_fetch_object($chapter_q);*/
	$query = db_select('textbook_companion_chapter');
	$query->fields('textbook_companion_chapter');
	$query->condition('id', $example_data->chapter_id);
	$query->range(0, 1);
	$result = $query->execute();
	$chapter_data = $result->fetchObject();
	if (!$chapter_data)
	{
		drupal_set_message('You do not have permission to delete this example.', 'error');
		drupal_goto('xcos-textbook-companion/code');
		return;
	} //!$chapter_data
	/*$preference_q = db_query("SELECT * FROM {xcos_textbook_companion_preference} WHERE id = %d LIMIT 1", $chapter_data->preference_id);
	$preference_data = db_fetch_object($preference_q);*/
	$query = db_select('textbook_companion_preference');
	$query->fields('textbook_companion_preference');
	$query->condition('id', $chapter_data->preference_id);
	$query->range(0, 1);
	$result = $query->execute();
	$preference_data = $result->fetchObject();
	if (!$preference_data)
	{
		drupal_set_message('You do not have permission to delete this example.', 'error');
		drupal_goto('xcos-textbook-companion/code');
		return;
	} //!$preference_data
	/*$proposal_q = db_query("SELECT * FROM {xcos_textbook_companion_proposal} WHERE id = %d AND uid = %d LIMIT 1", $preference_data->proposal_id, $user->uid);
	$proposal_data = db_fetch_object($proposal_q);*/
	$query = db_select('textbook_companion_proposal');
	$query->fields('textbook_companion_proposal');
	$query->condition('id', $preference_data->proposal_id);
	$query->condition('uid', $user->uid);
	$query->range(0, 1);
	$result = $query->execute();
	$proposal_data = $result->fetchObject();
	if (!$proposal_data)
	{
		drupal_set_message('You do not have permission to delete this example.', 'error');
		drupal_goto('xcos-textbook-companion/code');
		return;
	} //!$proposal_data
	/* deleting example files */
	if (xcos_delete_example($example_data->id))
	{
		drupal_set_message('Example deleted.', 'status');
		/* sending email */
		$email_to = $user->mail;
		$from = variable_get('xcos_textbook_companion_from_email', '');
		$bcc = variable_get('xcos_textbook_companion_emails', '');
		$cc = variable_get('xcos_textbook_companion_cc_emails', '');
		$params['example_deleted_user']['book_title'] = $preference_data->book;
		$params['example_deleted_user']['chapter_title'] = $chapter_data->name;
		$params['example_deleted_user']['example_number'] = $example_data->number;
		$params['example_deleted_user']['example_caption'] = $example_data->caption;
		$params['example_deleted_user']['user_id'] = $user->uid;
		$params['example_deleted_user']['headers'] = array(
			'From' => $from,
			'MIME-Version' => '1.0',
			'Content-Type' => 'text/plain; charset=UTF-8; format=flowed; delsp=yes',
			'Content-Transfer-Encoding' => '8Bit',
			'X-Mailer' => 'Drupal',
			'Cc' => $cc,
			'Bcc' => $bcc
		);
		if (!drupal_mail('xcos_textbook_companion', 'example_deleted_user', $email_to, language_default(), $params, $from, TRUE))
			drupal_set_message('Error sending email message.', 'error');
	} //delete_example($example_data->id)
	else
	{
		drupal_set_message('Error deleting example.', 'status');
	}
	drupal_goto('xcos-textbook-companion/code');
	return;
}
/******************************************************************************/
/************************** GENERAL FUNCTIONS *********************************/
/******************************************************************************/
function _list_of_book_titles()
{
	$book_titles = array(
		'0' => 'Please select...'
	);
	/*$book_titles_q = db_query("SELECT * FROM {xcos_textbook_companion_preference} WHERE approval_status = 1 OR approval_status = 3 ORDER BY book ASC");*/
	$query = db_select('textbook_companion_preference');
	$query->fields('textbook_companion_preference');
	$or = db_or();
	$or->condition('approval_status', 1);
	$or->condition('approval_status', 3);
	$query->condition($or);
	$query->orderBy('book', 'ASC');
	$book_titles_q = $query->execute();
	while ($book_titles_data = $book_titles_q->fetchObject())
	{
		$book_titles[$book_titles_data->id] = $book_titles_data->book . ' (Written by ' . $book_titles_data->author . ')';
	} //$book_titles_data = $book_titles_q->fetchObject()
	return $book_titles;
}
/*function _list_of_xcos_book_dependency_files($book_id)
{
	global $user;
	$book_dependency_files = array();
	$book_dependency_files_class = array();
	$query = db_select('textbook_companion_dependency_files');
	$query->fields('xcos_textbook_companion_dependency_files');
	$query->condition('preference_id', $book_id);
	$query->orderBy('filename', 'ASC');
	$book_dependency_files_q = $query->execute();
	while ($book_dependency_files_data = $book_dependency_files_q->fetchObject())
	{
		$temp_caption = '';
		if ($book_dependency_files_data->caption)
		{
			$book_dependency_files[$book_dependency_files_data->id] = l($book_dependency_files_data->filename, 'download/dependency/' . $book_dependency_files_data->id, array(
				'attributes' => array(
					'class' => 'title'
				)
			)) . $temp_caption;
			$book_dependency_files_class[$book_dependency_files_data->id] = $book_dependency_files_data->preference_id;
		} //$book_dependency_files_data->caption
	} //$book_dependency_files_data = $book_dependency_files_q->fetchObject()
	return array(
		$book_dependency_files,
		$book_dependency_files_class
	);
}*/

function xcos_ajax_chapter_name_callback($form, $form_state){

  $pref_id = $form_state['values']['pref_id'];
  $chapter_number = $form_state['values']['number'];
  //var_dump($pref_id . ' ' . $chapter_number);die;
  $example_number_options =array(
  	'Select' => 'Select');
  $commands = array(); 
  $chapter_number_q = db_query("SELECT * FROM textbook_companion_chapter WHERE preference_id = :pref_id AND number = :number",array(':pref_id'=> $pref_id,':number' => $chapter_number));
      //$chapter_name_q = db_query($query, $arg);
      $chapter_number_data = $chapter_number_q->fetchObject();
      //var_dump($chapter_number_data);die;
      $example_number_query = db_query("SELECT * FROM textbook_companion_example where chapter_id = :chapter_id", array(":chapter_id" => $chapter_number_data->id));
      while($example_number_data = $example_number_query->fetchObject()){
      	$example_number_options[$example_number_data->number] = $example_number_data->number;
      }
      if ($chapter_number_q->rowCount() > 0) {
      //$form['name']['#default_value'] = $chapter_name_data->fetchObject()->name; 
      $form['name']['#value'] = $chapter_number_data->name;    
      $form['name']['#readonly'] = TRUE;
      $form['name']['#attributes']['readonly'] = 'readonly';
      $commands[] = ajax_command_replace("#ajax-chapter-name-replace", drupal_render($form['name'])); 
      $form['example_number']['#options']  = $example_number_options;
      $form['example_number']['#readonly'] = TRUE;
      $form['example_number']['#attributes']['readonly'] = 'readonly';
      $commands[] = ajax_command_replace("#ajax-example-number-replace", drupal_render($form['example_number'])); 
      }else{
      $form['name']['#value'] = ' ';
      $form['name']['#readonly'] = FALSE;
      //$form['name']['#attributes']['readonly'] = 'readonly';
      $commands[] = ajax_command_replace("#ajax-chapter-name-replace", drupal_render($form['name']));
    
    }
      return array('#type' => 'ajax', '#commands' => $commands);

}
function xcos_ajax_example_caption_callback($form, $form_state){

  $pref_id = $form_state['values']['pref_id'];
  $chapter_number = $form_state['values']['number'];
  $example_number = $form_state['values']['example_number'];
  //var_dump($pref_id . ' ' . $chapter_number);die;
  $example_number_options =array(
  	'Select' => 'Select');
  $commands = array(); 
  $chapter_number_q = db_query("SELECT * FROM textbook_companion_chapter WHERE preference_id = :pref_id AND number = :number",array(':pref_id'=> $pref_id,':number' => $chapter_number));
      //$chapter_name_q = db_query($query, $arg);
      $chapter_number_data = $chapter_number_q->fetchObject();
      //var_dump($chapter_number_data);die;
      $example_number_query = db_query("SELECT * FROM textbook_companion_example where chapter_id = :chapter_id and number = :example_number", array(":chapter_id" => $chapter_number_data->id, ":example_number" => $example_number));
      $example_number_data = $example_number_query->fetchObject();
      	//$example_number_options[$example_number_data->number] = $example_number_data->number;
      if ($example_number_query->rowCount() > 0) {
      //$form['name']['#default_value'] = $chapter_name_data->fetchObject()->name; 
      $form['example_caption']['#value'] = $example_number_data->caption;    
      $commands[] = ajax_command_replace("#ajax-example-caption-replace", drupal_render($form['example_caption']));  
      }else{
      $form['example_caption']['#value'] = ' ';
      $form['example_caption']['#readonly'] = FALSE;
      //$form['name']['#attributes']['readonly'] = 'readonly';
      $commands[] = ajax_command_replace("#ajax-example-caption-replace", drupal_render($form['example_caption']));
    
    }
      return array('#type' => 'ajax', '#commands' => $commands);

}
