<?php
/* Textbook Companion Proposal */
function xcos_textbook_companion_proposal_all()
{
	global $user;
	$page_content = "";
	if (!$user->uid)
	{
		drupal_set_message('It is mandatory to login on this website to access the proposal form', 'error');
		drupal_goto('');
		return;
	} //!$user->uid
	/* check if user has already submitted a proposal */
	/*$proposal_q = db_query("SELECT * FROM {xcos_textbook_companion_proposal} WHERE uid = %d ORDER BY id DESC LIMIT 1", $user->uid);*/
	$query = db_select('xcos_textbook_companion_proposal');
	$query->fields('xcos_textbook_companion_proposal');
	$query->condition('uid', $user->uid);
	$query->orderBy('id', 'DESC');
	$query->range(0, 1);
	$proposal_q = $query->execute();
	if ($proposal_q)
	{
		if ($proposal_data = $proposal_q->fetchObject())
		{
			switch ($proposal_data->proposal_status)
			{
				case 0:
					drupal_set_message(t('We have already received your proposal. We will get back to you soon.'), 'status');
					drupal_goto('');
					return;
					break;
				case 1:
					drupal_set_message(t('Your proposal has been approved. Please go to ' . l('Code Submission', 'xcos-textbook-companion/code') . ' to upload your code'), 'status');
					drupal_goto('');
					return;
					break;
				case 2:
					drupal_set_message(t('Your proposal has been disapproved. Please create another proposal below.'), 'error');
					break;
				case 3:
					drupal_set_message(t('Congratulations! You have completed your last xcos book proposal. You can create another proposal below.'), 'status');
					break;
				default:
					drupal_set_message(t('Invalid proposal state. Please contact site administrator for further information.'), 'error');
					drupal_goto('');
					return;
					break;
			} //$proposal_data->proposal_status
		} //$proposal_data = $proposal_q->fetchObject()
	} //$proposal_q
	$book_proposal_form = drupal_get_form("book_proposal_form");
	$page_content .= drupal_render($book_proposal_form);
	// drupal_goto("aicte_proposal");
	return $page_content;
}
function book_proposal_form($form, &$form_state)
{
	global $user;
	$form = array();
	$state =  _xcos_tbc_list_of_states();
	$selected_state = isset($form_state['values']['all_state'])?$form_state['values']['all_state']:key($state);
	$district = _xcos_tbc_list_of_district();
	$selected_district = isset($form_state['values']['district'])?$form_state['values']['district']:key($district);
	$city = _xcos_tbc_list_of_cities();
	$selected_city = isset($form_state['values']['city'])?$form_state['values']['city']:key($city);
	$pincode = _xcos_tbc_list_of_city_pincode();
	$selected_pincode = isset($form_state['values']['picode'])?$form_state['values']['pincode']:key($pincode);

	$form['imp_notice'] = array(
		'#type' => 'item',
		'#markup' => '<font color="red"><b>Please fill up this form carefully as the details entered here will be exactly written in the Textbook Companion and also follow the additional guidelines.</b></font>'
	);
	$form['guidelines'] = array(
		'#type' => 'fieldset',
		'#title' => t('Guidelines'),
		'#attributes' => array(
			'style' => 'font-weight: bold'
		),
		'#collapsible' => TRUE,
		'#collapsed' => FALSE
	);
	$form['guidelines']['book'] = array(
		'#type' => 'item',
		'#required' => TRUE,
		'#markup' => '<ul style="list-style-type:disc;font-weight: normal">
               <li>All the fields are compulsory</li>
               <li>Please make sure that the book proposed by you has at least 25 examples which include numerical computations and which can be coded in Xcos</li>           
               <li>Make sure the book you propose is not already in a completed book or a book in progress list for Xcos TBC.</li>
               <li>You will be intimated about the approval or rejection of your suggestion via e-mail</li>

</ul> '
	);
	$form['full_name'] = array(
		'#type' => 'textfield',
		'#title' => t('Full Name'),
		'#size' => 30,
		'#maxlength' => 50,
		'#required' => TRUE
	);
	$form['email_id'] = array(
		'#type' => 'textfield',
		'#title' => t('Email'),
		'#size' => 30,
		'#value' => $user->mail,
		'#disabled' => TRUE
	);
	$form['mobile'] = array(
		'#type' => 'textfield',
		'#title' => t('Mobile No.'),
		'#size' => 30,
		'#maxlength' => 15,
		'#required' => TRUE
	);
	$form['gender'] = array(
		'#type' => 'radios',
		'#title' => t('Gender'),
		'#options' => array(
			'M' => 'Male',
			'F' => 'Female'
		),
		'#required' => TRUE
	);
	/*$form['how_project'] = array(
	'#type' => 'select',
	'#title' => t('How did you come to know about this project'),
	'#options' => array('Scilab Website' => 'Scilab Website',
	'Friend' => 'Friend',
	'Professor/Teacher' => 'Professor/Teacher',
	'Mailing List' => 'Mailing List',
	'Poster in my/other college' => 'Poster in my/other college',
	'Others' => 'Others'),
	'#required' => TRUE,
	);*/
	/*$form['course'] = array(
		'#type' => 'textfield',
		'#title' => t('Course'),
		'#size' => 30,
		'#maxlength' => 50,
		'#required' => TRUE
	);*/
	$form['branch'] = array(
		'#type' => 'select',
		'#title' => t('Department/Branch'),
		'#options' => _xcos_tbc_list_of_departments(),
		'#required' => TRUE
	);
	$form['university'] = array(
		'#type' => 'textfield',
		'#title' => t('Institution/Organisation name'),
		'#size' => 30,
		'#maxlength' => 50,
		'#required' => TRUE
	);
	$form['country'] = array(
		'#type' => 'select',
		'#title' => t('Country'),
		'#options' => array(
			'India' => 'India',
			//'Others' => 'Others'
		),
		'#default_value' => 'India',
		'#prefix' => '<div id="ajax-country-list-replace">',
		'#suffix' => '</div>',
		'#ajax' => array(
			'callback' => 'ajax_get_city_list_callback',
			),
		'#required' => TRUE,
		'#tree' => TRUE,
		'#validated' => TRUE
	);
	$form['other_country'] = array(
		'#type' => 'textfield',
		'#title' => t('Other than India'),
		'#size' => 100,
		'#attributes' => array(
			'placeholder' => t('Enter your country name')
		),
		'#states' => array(
			'visible' => array(
				':input[name="country"]' => array(
					'value' => 'Others'
				)
			)
		)
	);
	$form['other_state'] = array(
		'#type' => 'textfield',
		'#title' => t('State other than India'),
		'#size' => 100,
		'#attributes' => array(
			'placeholder' => t('Enter your state/region name')
		),
		'#states' => array(
			'visible' => array(
				':input[name="country"]' => array(
					'value' => 'Others'
				)
			)
		)
	);
	$form['other_city'] = array(
		'#type' => 'textfield',
		'#title' => t('City other than India'),
		'#size' => 100,
		'#attributes' => array(
			'placeholder' => t('Enter your city name')
		),
		'#states' => array(
			'visible' => array(
				':input[name="country"]' => array(
					'value' => 'Others'
				)
			)
		)
	);
	$form['all_state'] = array(
		'#type' => 'select',
		'#title' => t('State/UT'),
		'#options' => _xcos_tbc_list_of_states(),
		//'#empty_value' => '0',
		//'#empty_option' => '- Select -',
		'#default_value' => '0',
		'#prefix' => '<div id="ajax-state-list-replace">',
		'#suffix' => '</div>',
		'#required' => TRUE,
		'#ajax' => array(
			'callback' => 'ajax_get_district_list_callback',
			),
		'#validated' => TRUE,
		'#states' => array(
			'visible' => array(
				':input[name="country"]' => array(
					'value' => 'India'
				)
			)
		)
	);
	$form['district'] = array(
		'#type' => 'select',
		'#title' => t('District/Area'),
		'#options' => _xcos_tbc_list_of_district($selected_state),
		'#empty_value' => '0',
		'#empty_option' => '- Select -',
		'#prefix' => '<div id="ajax-district-list-replace">',
		'#suffix' => '</div>',
		'#validated' => TRUE,
		'#required' => TRUE,
		'#ajax' => array(
			'callback' => 'ajax_get_city_list_callback',
			),
		'#states' => array(
			'invisible' => array(
				':input[name="all_state"]' => array(
					'value' => '0'
				)
			)
		)
	);
	$form['city'] = array(
		'#type' => 'select',
		'#title' => t('City/Area'),
		'#options' => _xcos_tbc_list_of_cities($selected_state, $selected_district),
		'#default_value' => '0',
		'#empty_value' => '0',
		'#empty_option' => '- Select -',
		'#prefix' => '<div id="ajax-city-list-replace">',
		'#suffix' => '</div>',
		'#validated' => TRUE,
		'#required' => TRUE,
		'#ajax' => array(
			'callback' => 'ajax_get_city_pincode_list_callback',
			),
		'#states' => array(
			'invisible' => array(
				':input[name="district"]' => array(
					'value' => '0'
				)
			)
		)
	);
	$form['pincode'] = array(
		'#type' => 'select',
		'#title' => t('Pincode'),
		'#options' => _xcos_tbc_list_of_city_pincode($selected_city, $selected_state, $selected_district),
		'#prefix' => '<div id="ajax-pincode-list-replace">',
		'#suffix' => '</div>',
		'#empty_value' => '0',
		'#empty_option' => '- Select -',
		'#required' => TRUE,
		'#validated' => TRUE,
		'#states' => array(
			'invisible' => array(
				':input[name="city"]' => array(
					'value' => '0'
				)
			)
		)
	);
	$form['faculty'] = array(
		'#type' => 'hidden',
		'#value' => 'None',
		'#title' => t('College Teacher/Professor'),
		'#size' => 30,
		'#maxlength' => 50,
		'#required' => TRUE
	);
	$form['faculty_email'] = array(
		'#type' => 'hidden',
		'#value' => 'None',
		'#title' => t('Teacher/Professor Email Id'),
		'#default_value' => '@email.com',
		'#size' => 30,
		'#maxlength' => 50
	);
	$form['reviewer'] = array(
		'#type' => 'hidden',
		'#value' => 'Scilab TBC Team',
		'#title' => t('Reviewer'),
		'#size' => 30,
		'#maxlength' => 50
	);
	$form['version'] = array(
		'#type' => 'select',
		'#title' => t('Scilab Version'),
		'#options' => array(
			'Scilab 6.1.0' => 'Scilab 6.1.0',
		),
		'#required' => TRUE,
		//'#description' => t('Input version format should be like 5.4 or 5.4.1'),
		/*'#size' => 10,
		'#maxlength' => 20*/
	);
	$form['expected_date_of_completion'] = array(
		'#type' => 'date_popup',
		'#title' => t('Expected Date of Completion'),
		'#date_label_position' => '',
		//'#description' => '<span style="color:red">Please select a date within September 27, 2020</span>',
		'#default_value' => '',
		'#date_format' => 'd-m-Y',
		//'#date_increment' => 0,
		//'#minDate' => '+0',
		//'#date_year_range' => '0 : +0',
		'#datepicker_options' => array('minDate' => 0),
		'#required' => TRUE
	);
	$form['operating_system'] = array(
		'#type' => 'select',
		'#title' => t('Operating System'),
		'#options' => array(
			'Windows 32/64-bit Vista/7/8/10' => 'Windows 32/64-bit Vista/7/8/10',
			'Linux 32/64-bit x86 Ubuntu 16.04 or above' => 'Linux 32/64-bit x86 Ubuntu 16.04 or above',
			'MacOS 10.7 (OS X Lion) or newer' => 'MacOS 10.7 (OS X Lion) or newer'
		),
		'#required' => TRUE,
		/*'#size' => 30,
		'#maxlength' => 50*/
	);
	/***************************************************************************/
	$form['tbc_title_list'] = array(
            '#type' => 'select',
            '#title' => t('Select a book from this list'),
            '#required' => TRUE,
            '#options' => _xcos_list_of_book_titles(),
            '#ajax' => array(
        	'callback' => 'ajax_scilab_tbc_pref_data',
        	),
        	'#suffix' => '<div id="ajax_tbc_pref_data_replace"></div>',
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Submit')
	);
	$form['dir_name'] = array(
		'#type' => 'hidden',
		'#value' => 'None'
	);
	return $form;
}
function ajax_scilab_tbc_pref_data($form, $form_state)
{
	$commands = array();
	$book_pref_id = $form_state['values']['tbc_title_list'];
	//var_dump(_tbc_pref_data_details($book_pref_id));die;
	if ($book_pref_id != 0)
	{
		$commands[] = ajax_command_html('#ajax_tbc_pref_data_replace', _tbc_pref_data_details($book_pref_id));
	} //$case_study_project_default_value != 0
	else
	{
		//$commands[] = ajax_command_html('#ajax_tbc_pref_data_replace', _tbc_pref_data_details($book_pref_id));
		//$commands[] = ajax_command_data('#ajax_selected_case_study', 'form_state_value_select', $form_state['values']['case_study_project']);
	}
	return array(
		'#type' => 'ajax',
		'#commands' => $commands
	);
}

function _tbc_pref_data_details($pref_id){
	$return_html = "";
	$query = db_select('textbook_companion_preference');
	$query->fields('textbook_companion_preference');
	$query->condition('id', $pref_id);
	$preference_q = $query->execute();
	$preference_data = $preference_q->fetchObject();
	$return_html .= '<strong>Title of the book:</strong> ' . $preference_data->book . '<br />';
	$return_html .= '<strong>Author:</strong> ' . $preference_data->author . '<br />';
	$return_html .= '<strong>Edition:</strong> ' . $preference_data->edition . '<br />';			
	$return_html .= '<strong>Year:</strong> ' . $preference_data->year . '<br />';		
	$return_html .= '<strong>ISBN:</strong> ' . $preference_data->isbn . '<br />';
	return $return_html;
}

function book_proposal_form_validate($form, &$form_state)
{
/* mobile */
	if (!preg_match('/^[0-9\ \+]{0,15}$/', $form_state['values']['mobile']))
		form_set_error('mobile', t('Invalid mobile number'));
	/* edition */
	/*if (!preg_match('/^[1-9][0-9]{0,1}$/', $form_state['values']['edition1']))
		form_set_error('edition1', t('Invalid edition for Book Preference 1'));*/
	/* year of publication */
	/*if (!preg_match('/^[1-3][0-9][0-9][0-9]$/', $form_state['values']['year1']))
		form_set_error('year1', t('Invalid year of pulication for Book Preference 1'));
*/	/* year of publication */
	/*$cur_year = date('Y');
	if ((int) $form_state['values']['year1'] > $cur_year)
		form_set_error('year1', t('Year of pulication should be not in the future for Book Preference 1'));
	/* isbn */
	/*if (!preg_match('/^[0-9\-xX]+$/', $form_state['values']['isbn1']))
		form_set_error('isbn1', t('Invalid ISBN for Book Preference 1'));
	if (!preg_match('/^\d+(?:\.\d+)+/', $form_state['values']['version']))
		form_set_error('version', t('Please enter correct version in speacifed format'));*/
	list($d, $m, $y) = explode('-', $form_state['values']['expected_date_of_completion']);
	$d = (int) $d;
	$m = (int) $m;
	$y = (int) $y;
	//$completion_date = strtotime($form_state['values']['expected_date_of_completion']);
	//$last_date = strtotime("27-09-2020");
	//if ($completion_date > $last_date)
		//form_set_error('expected_date_of_completion', t('Please select a date within September 27, 2020'));
	if ($form_state['values']['country'] == 'Others')
	{
		if ($form_state['values']['other_country'] == '')
		{
			form_set_error('other_country', t('Enter country name'));
			// $form_state['values']['country'] = $form_state['values']['other_country'];
		} //$form_state['values']['other_country'] == ''
		else
		{
			$form_state['values']['country'] = $form_state['values']['other_country'];
		}
		if ($form_state['values']['other_state'] == '')
		{
			form_set_error('other_state', t('Enter state name'));
			// $form_state['values']['country'] = $form_state['values']['other_country'];
		} //$form_state['values']['other_state'] == ''
		else
		{
			$form_state['values']['all_state'] = $form_state['values']['other_state'];
		}
		if ($form_state['values']['other_city'] == '')
		{
			form_set_error('other_city', t('Enter city name'));
			// $form_state['values']['country'] = $form_state['values']['other_country'];
		} //$form_state['values']['other_city'] == ''
		else
		{
			$form_state['values']['city'] = $form_state['values']['other_city'];
		}
	} //$form_state['values']['country'] == 'Others'
	else
	{
		if ($form_state['values']['country'] == '')
		{
			form_set_error('country', t('Select country name'));
			// $form_state['values']['country'] = $form_state['values']['other_country'];
		} //$form_state['values']['country'] == ''
		if ($form_state['values']['all_state'] == '0')
		{
			form_set_error('all_state', t('Select state name'));
			// $form_state['values']['country'] = $form_state['values']['other_country'];
		} //$form_state['values']['all_state'] == ''
		if ($form_state['values']['district'] == '0')
		{
			form_set_error('district', t('Select district name'));
		}
		if ($form_state['values']['city'] == '0')
		{
			form_set_error('city', t('Select city name'));
			// $form_state['values']['country'] = $form_state['values']['other_country'];
		} //$form_state['values']['city'] == ''
		if ($form_state['values']['pincode'] == '0')
		{
			form_set_error('city', t('Select pincode'));
		}
	}

	return;
}
function book_proposal_form_submit($form, &$form_state, $directory_name = NULL)
{
	global $user;
	$root_path = textbook_companion_samplecode_path();
	$selections = variable_get("aicte_" . $user->uid, "");
	$tbc_to_emails = variable_get("textbook_companion_emails_all");
	$book_pref_id = $form_state['values']['tbc_title_list'];
	$query = db_select('textbook_companion_preference');
	$query->fields('textbook_companion_preference');
	$query->condition('id', $book_pref_id);
	$preference_q = $query->execute();
	$preference_data = $preference_q->fetchObject();
	$query_pro = db_select('textbook_companion_proposal');
	$query_pro->fields('textbook_companion_proposal');
	$query_pro->condition('id', $preference_data->proposal_id);
	$pro_q = $query_pro->execute();
	$scilab_proposal_data = $pro_q->fetchObject();
	if (!$user->uid)
	{
		drupal_set_message('It is mandatory to login on this website to access the proposal form', 'error');
		return;
	} //!$user->uid
	/* completion date to timestamp */
	/*list($d, $m, $y) = explode('-', $form_state['values']['completion_date']);
	$completion_date_timestamp = mktime(0, 0, 0, $m, $d, $y);*/
	/* Reasons for suggesting a book*/
	/*if (isset($_POST['reason']))
	{
		if (!($form_state['values']['other_reason']))
		{
			$my_reason = implode(", ", $_POST['reason']);
		} //!($form_state['values']['other_reason'])
		else
		{
			$my_reason = implode(", ", $_POST['reason']);
			$my_reason = $my_reason . "-" . " " . $form_state['values']['other_reason'];
		}
	}*/ //isset($_POST['reason'])
	/************************************/
	$scilab_version = $form_state['values']['version'];
	$query = "
INSERT INTO {xcos_textbook_companion_proposal}
    (uid, approver_uid, full_name, mobile, gender, how_project, branch, university, country, district, city, pincode, state, faculty, reviewer, reference, completion_date ,creation_date, approval_date, proposal_status, message, scilab_version, operating_system, teacher_email, proposal_type, reason, samplefilepath, proposed_completion_date) VALUES (:uid, :approver_uid, :full_name, :mobile, :gender, :how_project, :branch, :university, :country,:district, :city, :pincode, :state,
:faculty, :reviewer, :reference,:completion_date, :creation_date, :approval_date, :proposal_status, :message, :scilab_version,
 :operating_system, :teacher_email, :proposal_type, :reason, :samplefilepath, :proposed_completion_date)
";
	// $result = db_query($query);
	$args = array(
		":uid" => $user->uid,
		":approver_uid" => 0,
		":full_name" => _tbc_sentence_case($form_state['values']['full_name']),
		":mobile" => $form_state['values']['mobile'],
		":gender" => $form_state['values']['gender'],
		":how_project" => 'Not available',
		":branch" => $form_state['values']['branch'],
		":university" => $form_state['values']['university'],
		":country" => $form_state['values']['country'],
		":district" => $form_state['values']['district'],
		":city" => $form_state['values']['city'],
		":pincode" => $form_state['values']['pincode'],
		":state" => $form_state['values']['all_state'],
		":faculty" => _tbc_sentence_case($form_state['values']['faculty']),
		":reviewer" => _tbc_sentence_case($form_state['values']['reviewer']),
		":reference" => strtolower($form_state['values']['reference']),
		":completion_date" => strtotime($form_state['values']['expected_date_of_completion']),
		":creation_date" => time(),
		":approval_date" => time(),
		":proposal_status" => 0,
		":message" => 'None',
		":scilab_version" => $scilab_version,
		":operating_system" => $form_state['values']['operating_system'],
		":teacher_email" => $form_state['values']['faculty_email'],
		":proposal_type" => $scilab_proposal_data->proposal_type,
		":reason" => $scilab_proposal_data->reason,
		":samplefilepath" => "",
		":proposed_completion_date" => strtotime($form_state['values']['expected_date_of_completion'])
	);
	$result = db_query($query, $args, array(
		'return' => Database::RETURN_INSERT_ID
	));
	if (!$result)
	{
		drupal_set_message(t('Error receiving your proposal. Please try again.'), 'error');
		return;
	} //!$result
	/* proposal id */
	$proposal_id = $result;
	/* inserting first book preference */
	$query = "INSERT INTO {xcos_textbook_companion_preference}
      (proposal_id, pref_number, book, author, isbn, publisher, edition, year, approval_status, nonaicte_book) VALUES
 (:proposal_id, :pref_number, :book, :author, :isbn, :publisher, :edition, :year, :approval_status, :nonaicte_book)";
		$args = array(
			":proposal_id" => $proposal_id,
			":pref_number" => 1,
			":book" => $preference_data->book,
			":author" => $preference_data->author,
			":isbn" => $preference_data->isbn,
			":publisher" => $preference_data->publisher,
			":edition" => $preference_data->edition,
			":year" => $preference_data->year,
			//":category" => $form_state['values']['book_category_1'],
			":approval_status" => 0,
			":nonaicte_book" => 1
		);
		$result = db_query($query, $args);
		if (!$result)
		{
			drupal_set_message(t('Error receiving your first book preference.'), 'error');
		} //!$result
	 //$form_state['values']['book1']
	/* sending email */
	$email_to = $user->mail;
	$from = variable_get('xcos_textbook_companion_from_email', '');
	$bcc = variable_get('xcos_textbook_companion_emails', '');
	$cc = variable_get('xcos_textbook_companion_cc_emails', '');
	$params['proposal_received']['proposal_id'] = $proposal_id;
	$params['proposal_received']['user_id'] = $user->uid;
	$params['proposal_received']['headers'] = array(
		'From' => $from,
		'MIME-Version' => '1.0',
		'Content-Type' => 'text/plain; charset=UTF-8; format=flowed; delsp=yes',
		'Content-Transfer-Encoding' => '8Bit',
		'X-Mailer' => 'Drupal',
		'Cc' => $cc,
		'Bcc' => $bcc
	);
	if (!drupal_mail('xcos_textbook_companion', 'proposal_received', $email_to, language_default(), $params, $from, TRUE))
		drupal_set_message('Error sending email message.', 'error');
	drupal_set_message(t('We have received your xcos book proposal. We will get back to you soon.'), 'status');
	drupal_goto('');
}

function ajax_get_city_list_callback($form, $form_state)
{
    $state_default_value = $form_state['values']['all_state'];
    $district_default_value = $form_state['values']['district'];
    if ($district_default_value != '0')
    {
        $form['city']['#options'] = _tbc_list_of_cities($state_default_value, $district_default_value);
        $commands[] = ajax_command_replace("#ajax-city-list-replace", drupal_render($form['city']));
        $form['pincode']['#options'] =  array('0' => '- Select -');
        $commands[] = ajax_command_replace("#ajax-pincode-list-replace", drupal_render($form['pincode']));
    }else{
        $form['city']['#options'] = array('0' => '- Select -');
        $commands[] = ajax_command_replace("#ajax-city-list-replace", drupal_render($form['city']));
    }
    return array(
        '#type' => 'ajax',
        '#commands' => $commands
    );
}
function ajax_get_district_list_callback($form, $form_state)
{
    $state_default_value = $form_state['values']['all_state'];
    if ($state_default_value != '0')
    {
        $form['district']['#options'] = _tbc_list_of_district($state_default_value);
        $commands[] = ajax_command_replace("#ajax-district-list-replace", drupal_render($form['district']));
        $form['pincode']['#options'] =  array('0' =>'- Select -');
        $commands[] = ajax_command_replace("#ajax-pincode-list-replace", drupal_render($form['pincode']));
        $form['city']['#options'] = array('0' => '- Select -');
        $commands[] = ajax_command_replace("#ajax-city-list-replace", drupal_render($form['city']));
    }else{
        $form['district']['#options'] = array('0' => '- Select -');
        $commands[] = ajax_command_replace("#ajax-district-list-replace", drupal_render($form['district']));
        $form['pincode']['#options'] =  array('0' =>'- Select -');
        $commands[] = ajax_command_replace("#ajax-pincode-list-replace", drupal_render($form['pincode']));
        $form['city']['#options'] = array('0' => '- Select -');
        $commands[] = ajax_command_replace("#ajax-city-list-replace", drupal_render($form['city']));
    }
    return array(
        '#type' => 'ajax',
        '#commands' => $commands
    );
}
function ajax_get_city_pincode_list_callback($form, $form_state)
{
    $city_default_value = $form_state['values']['city'];
    $state_default_value = $form_state['values']['all_state'];
    $district_default_value = $form_state['values']['district'];
    if ($city_default_value != '0')
    {
        $form['pincode']['#options'] = _tbc_list_of_city_pincode($city_default_value,$state_default_value,$district_default_value);
        $commands[] = ajax_command_replace("#ajax-pincode-list-replace", drupal_render($form['pincode']));
    }else{
        $form['pincode']['#options'] =  array('0' => '- Select -');
        $commands[] = ajax_command_replace("#ajax-pincode-list-replace", drupal_render($form['pincode']));
    }
    return array(
        '#type' => 'ajax',
        '#commands' => $commands
    );
}