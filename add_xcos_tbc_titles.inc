<?php
function add_xcos_tbc_titles_form($form, &$form_state)
{
	global $user;
	/************************ start approve book details ************************/
	if ($user->uid == 0)
	{
		$msg = drupal_set_message(t('It is mandatory to ' . l('login', 'user') . ' on this website to add the xcos tbc titles. If you are new user please create a new account first.'), 'error');
		drupal_goto('user');
		return $msg;
	} //$user->uid == 0
	$form['#attributes'] = array(
		'enctype' => "multipart/form-data"
	);
	$form['new_tbc_title_name'] = array(
		'#type' => 'textfield',
		'#title' => t('Enter the name of the scilab tbc title'),
		'#size' => 500,
		'#maxlength' => 500,
		'#required' => TRUE
	);
	$form['scilab_pref_id'] = array(
		'#type' => 'textfield',
		'#title' => t('Enter the Preference id of the scilab tbc title'),
		'#size' => 250,
		'#maxlength' => 250,
		'#required' => TRUE
	);
	
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Submit')
	);
	return $form;
}

function add_xcos_tbc_titles_form_submit($form, &$form_state) { 
	global $user;
	$v = $form_state["values"];
	$result = "INSERT INTO {list_of_xcos_tbc_titles}
	(
	tbc_name,
	scilab_tbc_pref_id
	)VALUES
	(
	:tbc_name,
	:scilab_tbc_pref_id
	)";
	$args = array(
		":tbc_name" => $v['new_tbc_title_name'],
		":scilab_tbc_pref_id" => $v['scilab_pref_id']
	);
	$result1 = db_query($result, $args, array(
		'return' => Database::RETURN_INSERT_ID
	));
	drupal_set_message(t('TBC title added successfully'), 'status');
}