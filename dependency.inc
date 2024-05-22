<?php
// $Id$
function upload_dependency_form($form, &$form_state)
{
	global $user;
	/************************ start approve book details ************************/
	/*$proposal_q = db_query("SELECT * FROM {xcos_textbook_companion_proposal} WHERE uid = %d ORDER BY id DESC LIMIT 1", $user->uid);
	$proposal_data = db_fetch_object($proposal_q);*/
	$query = db_select('textbook_companion_proposal');
	$query->fields('textbook_companion_proposal');
	$query->condition('uid', $user->uid);
	$query->orderBy('id', 'DESC');
	$query->range(0, 1);
	$result = $query->execute();
	$proposal_data = $result->fetchObject();
	if (!$proposal_data)
	{
		drupal_set_message("Please submit a " . l('proposal', 'book_proposal') . ".", 'error');
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
				drupal_set_message(t('Your proposal has been disapproved. Please create another proposal ' . l('here', 'book_proposal') . '.'), 'error');
				drupal_goto('');
				return;
				break;
			case 3:
				drupal_set_message(t('Congratulations! You have completed your last book proposal. You have to create another proposal ' . l('here', 'book_proposal') . '.'), 'status');
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
	$query->condition('proposal_id', $proposal_data->id);
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
	$form['#attributes'] = array(
		'enctype' => "multipart/form-data"
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
	$form['existing_depfile'] = array(
		'#type' => 'item',
		'#markup' => _list_existing_dependency($preference_data->id),
		'#title' => t('List of existing dependency files for this book')
	);
	$form['depfile'] = array(
		'#type' => 'fieldset',
		'#title' => t('Upload Dependency Files'),
		'#collapsible' => FALSE,
		'#collapsed' => FALSE
	);
	$form['depfile']['depfile1'] = array(
		'#type' => 'file',
		'#title' => t('Upload dependency file'),
		'#description' => t("Allowed file extensions : ") . variable_get('xcos_textbook_companion_dependency_extensions', '')
	);
	$form['depfile']['depfile1_caption'] = array(
		'#type' => 'textfield',
		'#title' => t('Caption for dependency file'),
		'#size' => 15,
		'#maxlength' => 100,
		'#required' => TRUE
	);
	$form['depfile']['depfile1_description'] = array(
		'#type' => 'textarea',
		'#title' => t('Brief Description of the dependency file')
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Submit')
	);
	$form['cancel'] = array(
		'#type' => 'markup',
		'#value' => l(t('Back'), 'xcos_textbook_companion/code/upload')
	);
	return $form;
}
function upload_dependency_form_validate($form, &$form_state)
{
	global $user;
	/* get approved book details */
	/*$book_q = db_query("SELECT pro.id as pro_id, pre.id as pre_id, pre.book as pre_book, pro.full_name as pro_full_name FROM {xcos_textbook_companion_proposal} pro JOIN {xcos_textbook_companion_preference} pre ON pro.id = pre.proposal_id WHERE pro.proposal_status = 1 AND pre.approval_status = 1 AND pro.uid = %d", $user->uid);
	$book_data = db_fetch_object($book_q);*/
	$query = db_select('textbook_companion_proposal', 'pro');
	$query->fields('pro', array(
		'id',
		'full_name'
	));
	$query->fields('pre', array(
		'id',
		'book'
	));
	$query->innerJoin('xcos_textbook_companion_preference', 'pre', 'pro.id = pre.proposal_id');
	$query->condition('pro.proposal_status', 1);
	$query->condition('pre.approval_status', 1);
	$query->condition('pro.uid', $user->uid);
	$result = $query->execute();
	$book_data = $result->fetchObject();
	if (isset($_FILES['files']))
	{
		/* check for valid filename extensions */
		$allowed_extensions = explode(',', variable_get('xcos_textbook_companion_dependency_extensions', ''));
		foreach ($_FILES['files']['name'] as $file_form_name => $file_name)
		{
			if ($file_name)
			{
				$temp_extension = end(explode('.', strtolower($_FILES['files']['name'][$file_form_name])));
				if (!in_array($temp_extension, $allowed_extensions))
					form_set_error($file_form_name, t('Only ' . variable_get('xcos_textbook_companion_dependency_extensions', '') . ' extensions can be uploaded.'));
				if ($_FILES['files']['size'][$file_form_name] <= 0)
					form_set_error($file_form_name, t('File size cannot be zero.'));
				/* check if file already exists */
				/*$dep_exists_data = db_fetch_object(db_query("SELECT * FROM {xcos_textbook_companion_dependency_files} WHERE filename = '%s'", $_FILES['files']['name'][$file_form_name]));*/
				$query = db_select('textbook_companion_dependency_files');
				$query->fields('xcos_textbook_companion_dependency_files');
				$query->condition('filename', $_FILES['files']['name'][$file_form_name]);
				$result = $query->execute();
				$dep_exists_data = $result->fetchObject();
				if ($dep_exists_data)
					form_set_error($file_form_name, t('Dependency file with the same name has already been uploaded in this or some other book. Please rename the file and try again.'));
				/* check if valid file name */
				if (!xcos_textbook_companion_check_valid_filename($_FILES['files']['name'][$file_form_name]))
					form_set_error($file_form_name, t('Invalid file name specified. Only alphabets, numbers and underscore is allowed as a valid filename.'));
			} //$file_name
		} //$_FILES['files']['name'] as $file_form_name => $file_name
	} //isset($_FILES['files'])
}
function upload_dependency_form_submit($form, &$form_state)
{
	global $user;
	$root_path = xcos_textbook_companion_path();
	/* get approved book details */
	/*$proposal_q = db_query("SELECT * FROM {xcos_textbook_companion_proposal} WHERE uid = %d ORDER BY id DESC LIMIT 1", $user->uid);
	$proposal_data = db_fetch_object($proposal_q);*/
	$query = db_select('textbook_companion_proposal');
	$query->fields('textbook_companion_proposal');
	$query->condition('uid', $user->uid);
	$query->orderBy('id', 'DESC');
	$query->range(0, 1);
	$result = $query->execute();
	$proposal_data = $result->fetchObject();
	if (!$proposal_data)
	{
		drupal_set_message("Please submit a " . l('proposal', 'book_proposal') . ".", 'error');
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
				drupal_set_message(t('Your proposal has been disapproved. Please create another proposal ' . l('here', 'book_proposal') . '.'), 'error');
				drupal_goto('');
				return;
				break;
			case 3:
				drupal_set_message(t('Congratulations! You have completed your last book proposal. You have to create another proposal ' . l('here', 'book_proposal') . '.'), 'status');
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
	$query->condition('proposal_id', $proposal_data->id);
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
	$preference_id = $preference_data->id;
	$dest_path = $preference_id . '/';
	if (!is_dir($root_path . $dest_path))
		mkdir($root_path . $dest_path);
	$dest_path .= 'DEPENDENCIES' . '/';
	if (!is_dir($root_path . $dest_path))
		mkdir($root_path . $dest_path);
	/* uploading dependencies */
	$file_upload_counter = 0;
	$dependency_ids = array();
	$dependency_names = array();
	foreach ($_FILES['files']['name'] as $file_form_name => $file_name)
	{
		if ($file_name)
		{
			/* uploading file */
			if (move_uploaded_file($_FILES['files']['tmp_name'][$file_form_name], $root_path . $dest_path . $_FILES['files']['name'][$file_form_name]))
			{
				/* for uploaded files making an entry in the database */
				/* db_query("INSERT INTO {xcos_textbook_companion_dependency_files} (preference_id, filename, filepath, filemime, filesize, caption, description, timestamp)
				VALUES (%d, '%s', '%s', '%s', %d, '%s', '%s', %d)",
				$preference_id,
				$_FILES['files']['name'][$file_form_name],
				$dest_path . $_FILES['files']['name'][$file_form_name],
				$_FILES['files']['type'][$file_form_name],
				$_FILES['files']['size'][$file_form_name],
				$form_state['values'][$file_form_name . '_caption'],
				$form_state['values'][$file_form_name . '_description'],
				time()
				);*/
				$query = "INSERT INTO {xcos_textbook_companion_dependency_files} (preference_id, filename, filepath, filemime, filesize, caption, 	description, timestamp)
          VALUES (:preference_id, :filename, :filepath, :filemime, :filesize, :caption, :description, :timestamp)";
				$args = array(
					":preference_id" => $preference_id,
					":filename" => $_FILES['files']['name'][$file_form_name],
					":filepath" => $dest_path . $_FILES['files']['name'][$file_form_name],
					":filemime" => $_FILES['files']['type'][$file_form_name],
					":filesize" => $_FILES['files']['size'][$file_form_name],
					":caption" => $form_state['values'][$file_form_name . '_caption'],
					":description" => $form_state['values'][$file_form_name . '_description'],
					":timestamp" => time()
				);
				$result = db_query($query, $args, array(
					'return' => Database::RETURN_INSERT_ID
				));
				drupal_set_message($file_name . ' uploaded successfully.', 'status');
				//$dependency_ids[] = db_last_insert_id('xcos_textbook_companion_dependency_files', 'id');
				$dependency_ids[] = $result;
				$dependency_names[] = $_FILES['files']['name'][$file_form_name];
				$file_upload_counter++;
			} //move_uploaded_file($_FILES['files']['tmp_name'][$file_form_name], $root_path . $dest_path . $_FILES['files']['name'][$file_form_name])
			else
			{
				drupal_set_message('Error uploading dependency : ' . $dest_path . '/' . $_FILES['files']['name'][$file_form_name], 'error');
			}
		} //$file_name
	} //$_FILES['files']['name'] as $file_form_name => $file_name
	if ($file_upload_counter > 0)
	{
		drupal_set_message('Dependencies uploaded successfully.', 'status');
		/* sending email */
		$email_to = $user->mail;
		$from = variable_get('xcos_textbook_companion_from_email', '');
		$bcc = variable_get('xcos_textbook_companion_emails', '');
		$cc = variable_get('xcos_textbook_companion_cc_emails', '');
		$params['dependency_uploaded']['user_id'] = $user->uid;
		$params['dependency_uploaded']['headers'] = array(
			'From' => $from,
			'MIME-Version' => '1.0',
			'Content-Type' => 'text/plain; charset=UTF-8; format=flowed; delsp=yes',
			'Content-Transfer-Encoding' => '8Bit',
			'X-Mailer' => 'Drupal',
			'Cc' => $cc,
			'Bcc' => $bcc
		);
		$params['dependency_uploaded']['dependency_names'] = $dependency_names;
		$email_to = $user->mail;
		if (!drupal_mail('xcos_textbook_companion', 'dependency_uploaded', $email_to, language_default(), $params, $from, TRUE))
			drupal_set_message('Error sending email message.', 'error');
	} //$file_upload_counter > 0
	drupal_goto('xcos-textbook-companion/code/upload-dep');
}
function _list_existing_dependency($book_id)
{
	// $return_html = '<ul>';
	/*$query = "SELECT * FROM xcos_textbook_companion_dependency_files WHERE preference_id = %d ORDER BY filename ASC";
	$result = db_query($query, $book_id);*/
	$query = db_select('textbook_companion_dependency_files');
	$query->fields('xcos_textbook_companion_dependency_files');
	$query->condition('preference_id', $book_id);
	$query->orderBy('filename', 'ASC');
	$result = $query->execute();
	$headers = array(
		"File",
		"Action"
	);
	$rows = array();
	while ($row = $result->fetchObject())
	{
		$item = array(
			l($row->filename . $temp_caption, 'xcos-textbook-companion/download/dependency/' . $row->id),
			l("Delete", "xcos-textbook-companion/code/delete-dep/{$row->id}")
			//l("Edit", 'xcos-textbook-companion/code/edit-dep/' . $row->id) . " | " . l("Delete", "xcos-textbook-companion/code/delete-dep/{$row->id}")
		);
		array_push($rows, $item);
	} //$row = $result->fetchObject()
	$return_html .= theme("table", array(
		"headers" => $headers,
		"rows" => $rows
	));
	return $return_html;
}
/******************** edit dependency section ********************/
function edit_dependency_form($form_state, $dependency_id = 0)
{
	global $user;
	/************************ start approve book details ************************/
	/*$proposal_q = db_query("SELECT * FROM {xcos_textbook_companion_proposal} WHERE uid = %d ORDER BY id DESC LIMIT 1", $user->uid);
	$proposal_data = db_fetch_object($proposal_q);*/
	$query = db_select('textbook_companion_proposal');
	$query->fields('textbook_companion_proposal');
	$query->condition('uid', $user->uid);
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
	$query->condition('proposal_id', $proposal_data->id);
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
	/* fetching the default values */
	/*$query = "
	SELECT * FROM {xcos_textbook_companion_dependency_files}
	WHERE id = {$dependency_id}
	";
	$result = db_query($query);
	$row = db_fetch_object($result);*/
	$query_dep = db_select('textbook_companion_dependency_files');
	$query_dep->fields('xcos_textbook_companion_dependency_files');
	$query_dep->condition('id', $dependency_id);
	$result = $query_dep->execute();
	$row = $result->fetchObject();
	$form['#attributes'] = array(
		'enctype' => "multipart/form-data"
	);
	$form['depfile'] = array(
		'#type' => 'fieldset',
		'#title' => t('Upload New Dependency File'),
		'#collapsible' => FALSE,
		'#collapsed' => FALSE
	);
	$form['depfile']['depfile1'] = array(
		'#type' => 'file',
		'#title' => t('Upload dependency file'),
		'#description' => t("Allowed file extensions : ") . variable_get('xcos_textbook_companion_dependency_extensions', '')
	);
	$form['depfile']['depfile1_caption'] = array(
		'#type' => 'textfield',
		'#title' => t('Caption for dependency file'),
		'#size' => 15,
		'#maxlength' => 100,
		'#required' => TRUE,
		"#default_value" => $row->caption
	);
	$form['depfile']['depfile1_description'] = array(
		'#type' => 'textarea',
		'#title' => t('Brief Description of the dependency file'),
		"#default_value" => $row->description
	);
	$form["dependency_id"] = array(
		"#type" => "hidden",
		"#value" => $dependency_id
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Submit')
	);
	$form['cancel'] = array(
		'#type' => 'markup',
		'#value' => l(t('Back'), 'xcos-textbook-companion/code/upload-dep')
	);
	return $form;
}
function edit_dependency_form_validate($form, &$form_state)
{
	global $user;
	if (isset($_FILES['files']))
	{
		/* check for valid filename extensions */
		$allowed_extensions = explode(',', variable_get('xcos_textbook_companion_dependency_extensions', ''));
		foreach ($_FILES['files']['name'] as $file_form_name => $file_name)
		{
			if ($file_name)
			{
				$temp_extension = end(explode('.', strtolower($_FILES['files']['name'][$file_form_name])));
				if (!in_array($temp_extension, $allowed_extensions))
					form_set_error($file_form_name, t('Only ' . variable_get('xcos_textbook_companion_dependency_extensions', '') . ' extensions can be uploaded.'));
				if ($_FILES['files']['size'][$file_form_name] <= 0)
					form_set_error($file_form_name, t('File size cannot be zero.'));
				/* check if file already exists */
				/*$dep_exists_data = db_fetch_object(
				db_query(
				"SELECT * FROM {xcos_textbook_companion_dependency_files} WHERE filename = '%s' AND id != %d",
				$_FILES['files']['name'][$file_form_name],
				$form_state["values"]["dependency_id"]
				)
				);*/
				$query = db_select('textbook_companion_dependency_files');
				$query->fields('xcos_textbook_companion_dependency_files');
				$query->condition('filename', $_FILES['files']['name'][$file_form_name]);
				$query->condition('id', $form_state["values"]["dependency_id"], '<>');
				$result = $query->execute();
				$dep_exists_data = $result->fetchObject();
				if ($dep_exists_data)
					form_set_error($file_form_name, t('Dendency file with the same name has already been uploaded in this or some other book. Please rename the file and try again.'));
				/* check if valid file name */
				if (!xcos_textbook_companion_check_valid_filename($_FILES['files']['name'][$file_form_name]))
					form_set_error($file_form_name, t('Invalid file name specified. Only alphabets, numbers and underscore is allowed as a valid filename.'));
			} //$file_name
		} //$_FILES['files']['name'] as $file_form_name => $file_name
	} //isset($_FILES['files'])
}
function edit_dependency_form_submit($form, &$form_state)
{
	global $user;
	$root_path = xcos_textbook_companion_path();
	/* get approved book details */
	/*$proposal_q = db_query("SELECT * FROM {xcos_textbook_companion_proposal} WHERE uid = %d ORDER BY id DESC LIMIT 1", $user->uid);
	$proposal_data = db_fetch_object($proposal_q);*/
	$query = db_select('textbook_companion_proposal');
	$query->fields('textbook_companion_proposal');
	$query->condition('uid', $user->uid);
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
	$query->condition('proposal_id', $proposal_data->id);
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
	$preference_id = $preference_data->id;
	$dest_path = $preference_id . '/';
	if (!is_dir($root_path . $dest_path))
		mkdir($root_path . $dest_path);
	$dest_path .= 'DEPENDENCIES' . '/';
	if (!is_dir($root_path . $dest_path))
		mkdir($root_path . $dest_path);
	/* uploading dependencies */
	$file_upload_counter = 0;
	$dependency_ids = array();
	$dependency_names = array();
	/* deleting old dependency file if the filename changed */
	/*$query = "
	SELECT * FROM xcos_textbook_companion_dependency_files 
	WHERE id = %d
	";
	$result = db_query($query, $form_state["values"]["dependency_id"]);
	$row = db_fetch_object($result);*/
	$query = db_select('textbook_companion_dependency_files');
	$query->fields('xcos_textbook_companion_dependency_files');
	$query->condition('id', $form_state["values"]["dependency_id"]);
	$result = $query->execute();
	$row = $result->fetchObject();
	if ($row->filename != $_FILES["files"]["name"][$file_form_name] && $_FILES["files"]["name"][$file_form_name])
	{
		unlink($root_path . $dest_path . $row->filename);
	} //$row->filename != $_FILES["files"]["name"][$file_form_name] && $_FILES["files"]["name"][$file_form_name]
	foreach ($_FILES['files']['name'] as $file_form_name => $file_name)
	{
		if ($file_name)
		{
			/* 
			 * uploading file 
			 * file will be overwritten if same name
			 */
			if (move_uploaded_file($_FILES['files']['tmp_name'][$file_form_name], $root_path . $dest_path . $_FILES['files']['name'][$file_form_name]))
			{
				/* updating uploaded file entry in the database */
				/*db_query(
				"UPDATE {xcos_textbook_companion_dependency_files} SET
				filename = '%s', filepath = '%s', filemime = '%s', filesize = %d,
				caption = '%s', description = '%s', timestamp = %s
				WHERE id = %d",
				$_FILES['files']['name'][$file_form_name],
				$dest_path . $_FILES['files']['name'][$file_form_name],
				$_FILES['files']['type'][$file_form_name],
				$_FILES['files']['size'][$file_form_name],
				$form_state['values'][$file_form_name . '_caption'],
				$form_state['values'][$file_form_name . '_description'],
				time(),
				$form_state["values"]["dependency_id"]
				);*/
				$query = db_update('xcos_textbook_companion_dependency_files');
				$query->fields(array(
					'filename' => $_FILES['files']['name'][$file_form_name],
					'filepath' => $dest_path . $_FILES['files']['name'][$file_form_name],
					'filemime' => $_FILES['files']['type'][$file_form_name],
					'filesize' => $_FILES['files']['size'][$file_form_name],
					'caption' => $form_state['values'][$file_form_name . '_caption'],
					'description' => $form_state['values'][$file_form_name . '_description'],
					'timestamp' => time()
				));
				$query->condition('id', $form_state["values"]["dependency_id"]);
				$num_updated = $query->execute();
				drupal_set_message($file_name . ' uploaded successfully.', 'status');
				$dependency_ids[] = db_last_insert_id('xcos_textbook_companion_dependency_files', 'id');
				$dependency_names[] = $_FILES['files']['name'][$file_form_name];
				$file_upload_counter++;
			} //move_uploaded_file($_FILES['files']['tmp_name'][$file_form_name], $root_path . $dest_path . $_FILES['files']['name'][$file_form_name])
			else
			{
				drupal_set_message('Error uploading dependency : ' . $dest_path . '/' . $_FILES['files']['name'][$file_form_name], 'error');
			}
		} //$file_name
	} //$_FILES['files']['name'] as $file_form_name => $file_name
	if ($file_upload_counter > 0)
	{
		drupal_set_message('Dependencies uploaded successfully.', 'status');
		/* sending email */
		$from = variable_get('xcos_textbook_companion_from_email', '');
		$bcc = variable_get('xcos_textbook_companion_emails', '');
		$cc = variable_get('xcos_textbook_companion_cc_emails', '');
		$params['dependency_uploaded']['user_id'] = $user->uid;
		$params['dependency_uploaded']['dependency_names'] = $dependency_names;
		$params['dependency_uploaded']['headers'] = array(
			'From' => $from,
			'MIME-Version' => '1.0',
			'Content-Type' => 'text/plain; charset=UTF-8; format=flowed; delsp=yes',
			'Content-Transfer-Encoding' => '8Bit',
			'X-Mailer' => 'Drupal',
			'Cc' => $cc,
			'Bcc' => $bcc
		);
		$email_to = $user->mail;
		if (!drupal_mail('xcos_textbook_companion', 'dependency_uploaded', $email_to, language_default(), $params, $from, TRUE))
			drupal_set_message('Error sending email message.', 'error');
	} //$file_upload_counter > 0
	drupal_goto('xcos-textbook-companion/code/upload-dep');
}
function edit_dependency($dependency_id = 0)
{
	if ($dependency_id)
	{
		$page_content = "";
		$page_content .= drupal_get_form("edit_dependency_form", $dependency_id);
		return $page_content;
	} //$dependency_id
	else
	{
		drupal_goto("xcos-textbook-companion/code/upload-dep");
	}
}
/******************** delete dependency section ********************/
function delete_dependency($dependency_id = 0, $confirm = "")
{
	global $user;
	/************************ start approve book details ************************/
	/*$proposal_q = db_query("SELECT * FROM {xcos_textbook_companion_proposal} WHERE uid = %d ORDER BY id DESC LIMIT 1", $user->uid);
	$proposal_data = db_fetch_object($proposal_q);*/
	$query = db_select('textbook_companion_proposal');
	$query->fields('textbook_companion_proposal');
	$query->condition('uid', $user->uid);
	$query->orderBy('id', 'DESC');
	$query->range(0, 1);
	$result = $query->execute();
	$proposal_data = $result->fetchObject();
	if (!$proposal_data)
	{
		drupal_set_message("Please submit a " . l('proposal', 'book_proposal') . ".", 'error');
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
				drupal_set_message(t('Your proposal has been disapproved. Please create another proposal ' . l('here', 'book_proposal') . '.'), 'error');
				drupal_goto('');
				return;
				break;
			case 3:
				drupal_set_message(t('Congratulations! You have completed your last book proposal. You have to create another proposal ' . l('here', 'book_proposal') . '.'), 'status');
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
	$query->condition('proposal_id', $proposal_data->id);
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
	$page_content = "";
	if ($dependency_id && $confirm == "yes")
	{
		/* removing the dependency file from filesystem */
		/*$query = "
		SELECT * FROM {xcos_textbook_companion_dependency_files}
		WHERE id = {$dependency_id}
		";
		$result = db_query($query);
		$row = db_fetch_object($result);*/
		$query = db_select('textbook_companion_dependency_files');
		$query->fields('xcos_textbook_companion_dependency_files');
		$query->condition('id', $dependency_id);
		$result = $query->execute();
		$row = $result->fetchObject();
		if ($preference_data->id == $row->preference_id)
		{
			$root_path = xcos_textbook_companion_path();
			$dest_path = $row->preference_id . '/';
			$dest_path .= 'DEPENDENCIES' . '/';
			unlink($root_path . $dest_path . $row->filename);
			/* deleting entry in xcos_textbook_companion_dependency_files */
			/*$query = "
			DELETE FROM {xcos_textbook_companion_dependency_files}
			WHERE id = {$dependency_id}
			";
			db_query($query);*/
			$query = db_delete('xcos_textbook_companion_dependency_files');
			$query->condition('id', $dependency_id);
			$num_deleted = $query->execute();
			/* deleting entries from xcos_textbook_companion_example_dependency */
			/*$query = "
			DELETE FROM {xcos_textbook_companion_example_dependency}
			WHERE dependency_id = {$dependency_id}
			";
			db_query($query);*/
			$query = db_delete('xcos_textbook_companion_example_dependency');
			$query->condition('dependency_id', $dependency_id);
			$num_deleted = $query->execute();
			drupal_set_message("Dependency deleted successfully.");
			drupal_goto("xcos-textbook-companion/code/upload-dep");
		} //$preference_data->id == $row->preference_id
		else
		{
			drupal_set_message("Cannot delete other users dependency.", "error");
			drupal_goto("xcos-textbook-companion/code/upload-dep");
		}
	} //$dependency_id && $confirm == "yes"
	else if ($dependency_id)
	{
		/*$query = "
		SELECT * FROM {xcos_textbook_companion_dependency_files}
		WHERE id = %d
		";
		$result = db_query($query, $dependency_id);
		$row = db_fetch_object($result);*/
		$query = db_select('textbook_companion_dependency_files');
		$query->fields('xcos_textbook_companion_dependency_files');
		$query->condition('id', $dependency_id);
		$result = $query->execute();
		$row = $result->fetchObject();
		if ($preference_data->id == $row->preference_id)
		{
			$page_content .= "Are you sure you want to delete this dependency?<br><br>";
			$page_content .= "Dependency filename: <b>{$row->filename}</b><br><br>";
			/* fetching the examples linked to this dependency */
			/*$query = "
			SELECT * FROM xcos_textbook_companion_example_dependency dep
			LEFT JOIN xcos_textbook_companion_example exa ON exa.id = dep.example_id
			WHERE dep.id = %d
			";
			$result = db_query($query, $dependency_id);*/
			$query = db_select('textbook_companion_example_dependency', 'dep');
			$query->fields('dep');
			$query->leftJoin('textbook_companion_example', 'exa', 'exa.id = dep.example_id');
			$query->condition('dep.id', $dependency_id);
			$result = $query->execute();
			$list = "";
			while ($row = $result->fetchObject())
			{
				$list .= "<li>Example {$row->number}</li>";
			} //$row = $result->fetchObject()
			if ($list)
			{
				$page_content .= "List of examples linking to this dependency are:";
				$page_content .= "<ul>{$list}</ul>";
			} //$list
			$page_content .= l("Delete dependency", "xcos-textbook-companion/code/delete-dep/{$dependency_id}/yes");
			$page_content .= " | ";
			$page_content .= l("Cancel", "xcos-textbook-companion/code/upload-dep");
		} //$preference_data->id == $row->preference_id
		else
		{
			drupal_set_message("Cannot delete other users dependency.", "error");
			drupal_goto("xcos-textbook-companion/code/upload-dep");
		}
	} //$dependency_id
	else
	{
		drupal_goto("xcos-textbook-companion/code/upload-dep");
	}
	return $page_content;
}
