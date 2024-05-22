<?php
// $Id$
function list_chapters()
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
	//var_dump($proposal_data);die;
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
	if ($proposal_data->submited_all_examples_code == 1)
      {
      drupal_set_message(t('You have submited your all codes for this book to review, hence you can not upload more code, for any query please write us.'), 'status');
                drupal_goto('');
                return;       
      }
	if (!$preference_data)
	{
		drupal_set_message(t('Invalid Book Preference status. Please contact site administrator for further information.'), 'error');
		drupal_goto('');
		return;
	} //!$preference_data
	/************************ end approve book details **************************/
	if($proposal_data->follow_guidelines == 1){
	$return_html = '';
	$return_html .= '<strong>Title of the Book:</strong><br />' . $preference_data->book . '<br /><br />';
	$return_html .= '<strong>Contributor Name:</strong><br />' . $proposal_data->full_name . '<br /><br />';
	$return_html .= l('Upload Example Code', 'xcos-textbook-companion/code/upload') . '<br />';
	/* get chapter list */
	$chapter_rows = array();
	/*$chapter_q = db_query("SELECT * FROM {xcos_textbook_companion_chapter} WHERE preference_id = %d ORDER BY number ASC", $preference_data->id);*/
	$query = db_select('textbook_companion_chapter');
	$query->fields('textbook_companion_chapter');
	$query->condition('preference_id', $preference_data->id);
	$query->orderBy('number', 'ASC');
	$chapter_q = $query->execute();
	while ($chapter_data = $chapter_q->fetchObject())
	{
		/* get example list */
		/* $example_q = db_query("SELECT count(*) as example_count FROM {xcos_textbook_companion_example} WHERE chapter_id = %d", $chapter_data->id);
		$example_data = db_fetch_object($example_q);*/
		/*$query = db_select('textbook_companion_example');
		$query->addExpression('count(*)', 'example_count');
		//$query->condition('approval_status', 0);
		$query->condition('chapter_id', $chapter_data->id);
		$result = $query->execute();*/
		$result = db_query("SELECT count(*) as example_count FROM 
textbook_companion_example as e LEFT JOIN 
textbook_companion_example_files f on f.example_id = e.id 
WHERE e.chapter_id = :chapter_id and f.filename LIKE '%xcos%' ", array(
":chapter_id" => $chapter_data->id,
));
		$example_data = $result->fetchObject();
		$chapter_rows[] = array(
			$chapter_data->number,
			$chapter_data->name,
			$example_data->example_count,
			l('View', 'xcos-textbook-companion/code/list-examples/' . $chapter_data->id)
		);
	} //$chapter_data = $chapter_q->fetchObject()
	/* check if there are any chapters */
	if (!$chapter_rows)
	{
		drupal_set_message(t('No uploads found.'), 'status');
		return $return_html;
	} //!$chapter_rows
	$chapter_header = array(
		'Chapter No.',
		'Title of the Chapter',
		'Uploaded Examples',
		'Actions'
	);
	$return_html .= theme('table', array(
		'header' => $chapter_header,
		'rows' => $chapter_rows
	));
	$submited_all_example = drupal_get_form("all_example_submitted_check_form",$preference_data->id);
    $return_html .= drupal_render($submited_all_example);
	}
	else{
		drupal_goto('xcos-textbook-companion/follow-guidelines/' . $proposal_data->id);
	/*$return_html = '';
	$follow_guidelines = drupal_get_form("follow_guidelines_form",$preference_data->id);
    $return_html .= drupal_render($follow_guidelines);*/
	}	
	return $return_html;
}
function list_examples()
{
	global $user;
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
	/* get chapter details */
	$chapter_id = arg(3);
	/*$chapter_q = db_query("SELECT * FROM {xcos_textbook_companion_chapter} WHERE id = %d AND preference_id = %d LIMIT 1", $chapter_id, $preference_data->id);*/
	$query = db_select('textbook_companion_chapter');
	$query->fields('textbook_companion_chapter');
	$query->condition('id', $chapter_id);
	$query->condition('preference_id', $preference_data->id);
	$query->range(0, 1);
	$chapter_q = $query->execute();
	if ($chapter_data = $chapter_q->fetchObject())
	{
		$return_html = '<br />';
		$return_html .= '<strong>Title of the Book:</strong><br />' . $preference_data->book . '<br /><br />';
		$return_html .= '<strong>Contributor Name:</strong><br />' . $proposal_data->full_name . '<br /><br />';
		$return_html .= '<strong>Chapter Number:</strong><br />' . $chapter_data->number . '<br /><br />';
		$return_html .= '<strong>Title of the Chapter:</strong><br />' . $chapter_data->name . '<br />';
	} //$chapter_data = $chapter_q->fetchObject()
	else
	{
		drupal_set_message(t('Invalid chapter.'), 'error');
		drupal_goto('xcos-textbook-companion/code');
		return;
	}
	$return_html .= '<br />' . l('Back to Chapter List', 'xcos-textbook-companion/code');
	/* get example list */
	$example_rows = array();
	/*$example_q = db_query("SELECT * FROM {xcos_textbook_companion_example} WHERE chapter_id = %d ORDER BY
	CAST(SUBSTRING_INDEX(number, '.', 1) AS BINARY) ASC,
	CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(number , '.', 2), '.', -1) AS UNSIGNED) ASC,
	CAST(SUBSTRING_INDEX(SUBSTRING_INDEX(number , '.', -1), '.', 1) AS UNSIGNED) ASC", $chapter_id);*/
	$query = db_select('textbook_companion_example');
	$query->fields('textbook_companion_example');
	$query->condition('chapter_id', $chapter_id);
	$example_q = $query->execute();
	while ($example_data = $example_q->fetchObject())
	{
		/* approval status */
		$approval_status = '';
		switch ($example_data->approval_status)
		{
			case 0:
				$approval_status = 'Pending';
				break;
			case 1:
				$approval_status = 'Approved';
				break;
			case 2:
				$approval_status = 'Rejected';
				break;
		} //$example_data->approval_status
		/* example files */
		$example_files = '';
		/*$example_files_q = db_query("SELECT * FROM {xcos_textbook_companion_example_files} WHERE example_id = %d ORDER BY filetype", $example_data->id);*/
		$query = db_select('textbook_companion_example_files');
		$query->fields('textbook_companion_example_files');
		$query->condition('example_id', $example_data->id);
		$or = db_or();
		$or->condition('filetype', 'P');
		$or->condition('filetype','Q');
		$query->condition($or);
		$query->orderBy('filetype', 'ASC');
		$example_files_q = $query->execute();
		while ($example_files_data = $example_files_q->fetchObject())
		{
			$file_type = '';
			switch ($example_files_data->filetype)
			{
				case 'P':
					$file_type = 'Main or Source';
					break;
				case 'Q':
					$file_type = 'Result';
					break;
				case 'X':
					$file_type = 'xcos';
					break;
				default:
			} //$example_files_data->filetype
			$example_files .= l($example_files_data->filename, 'xcos-textbook-companion/download/file/' . $example_files_data->id) . ' (' . $file_type . ')<br />';
		} //$example_files_data = $example_files_q->fetchObject()
		/* dependency files */
		$dependency_files = '';
		$dependency_files_q = db_query("SELECT dependency.id as dependency_id, dependency.filename as dependency_filename
      FROM {xcos_textbook_companion_example_dependency} example_dependency JOIN {xcos_textbook_companion_dependency_files} dependency
      ON example_dependency.dependency_id = dependency.id
      WHERE example_dependency.example_id = :example_id", array(
			':example_id' => $example_data->id
		));
		/*$query = db_select('textbook_companion_example_dependency', 'example_dependency');
		$query->fields('dependency', array('id', 'filename'));
		$query->innerJoin('xcos_textbook_companion_dependency_files', 'dependency', 'example_dependency.dependency_id = dependency.id');
		$query->condition('example_dependency.example_id', $example_data->id);
		$dependency_files_q = $query->execute();*/
		while ($dependency_files_data = $dependency_files_q->fetchObject())
		{
			$dependency_files .= l($dependency_files_data->dependency_filename, 'xcos-textbook-companion/download/dependency/' . $dependency_files_data->dependency_id) . ' (Dependency)<br />';
		} //$dependency_files_data = $dependency_files_q->fetchObject()
		$example_files .= $dependency_files;
		if ($example_data->approval_status == 0)
		{
			$example_rows[] = array(
				'data' => array(
					$example_data->number,
					$example_data->caption,
					$approval_status,
					$example_files,
					l('Edit', 'xcos-textbook-companion/code/edit/' . $example_data->id)
					/*l('Edit', 'xcos-textbook-companion/code/edit/' . $example_data->id) . ' | ' . l('Delete', 'xcos-textbook-companion/code/delete/' . $example_data->id, array(
						'attributes' => array(
							'onClick' => 'return confirm("Are you sure you want to delete the example?")'
						)
					))*/
				),
				'valign' => 'top'
			);
		} //$example_data->approval_status == 0
		else
		{
			$example_rows[] = array(
				'data' => array(
					$example_data->number,
					$example_data->caption,
					$approval_status,
					$example_files,
					l('Download', 'xcos-textbook-companion/download/example/' . $example_data->id)
				),
				'valign' => 'top'
			);
		}
	} //$example_data = $example_q->fetchObject()
	$example_header = array(
		'Example No.',
		'Caption',
		'Status',
		'Files',
		'Action'
	);
	$return_html .= theme('table', array(
		'header' => $example_header,
		'rows' => $example_rows
	));
	return $return_html;
}

function all_example_submitted_check_form($form,&$form_state,$preference_id){
    $form = array();
    $form['all_example_submitted'] = array(
            '#type' => 'checkbox',
            '#title' => t(' I have submitted the required number of examples'),
            '#description' => 'Once you have submitted this option you are not able to upload more examples.',
            '#required' => TRUE,
        );
        $form['hidden_preference_id'] = array(
        '#type' => 'hidden',
        '#value' => $preference_id
    );
        $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit')
    );
        return $form;
}
function all_example_submitted_check_form_validate($form,&$form_state){
if ($form_state['values']['all_example_submitted'] != 1) {
    form_set_error('all_example_submitted', t('Please check the field if you are intrested to submit the all uploaded examples for review!'));
  }
  return;
}

function all_example_submitted_check_form_submit($form,&$form_state){
global $user;
if ($form_state['values']['all_example_submitted'] == 1) {
db_query('UPDATE textbook_companion_proposal SET submited_all_examples_code = 1 WHERE preference_id = :preference_id',array(':preference_id' => $form_state['values']['hidden_preference_id']));
 /* sending email */
 $query = ("SELECT id FROM textbook_companion_proposal WHERE preference_id= :preference_id");
 $args = array(":preference_id" => $form_state['values']['hidden_preference_id']);
 $proposal_data = db_query($query,$args);
 $proposal_data_result = $proposal_data->fetchObject();

    $email_to = $user->mail;
    $from = variable_get('xcos_textbook_companion_from_email', '');
        $bcc = variable_get('xcos_textbook_companion_emails', '');
        $cc = variable_get('xcos_textbook_companion_cc_emails', '');
    $param['all_code_submitted']['proposal_id'] = $proposal_data_result->id;
    $param['all_code_submitted']['user_id'] = $user->uid;
    $param['all_code_submitted']['headers'] = array(
        'From' => $from,
        'MIME-Version' => '1.0',
        'Content-Type' => 'text/plain; charset=UTF-8; format=flowed; delsp=yes',
        'Content-Transfer-Encoding' => '8Bit',
        'X-Mailer' => 'Drupal',
        'Cc' => $cc,
        'Bcc' => $bcc
    );
    if (!drupal_mail('xcos_textbook_companion', 'all_code_submitted', $email_to, language_default(), $param, $from, TRUE))
        drupal_set_message('Error sending email message.', 'error');
}
}

function follow_guidelines_form($form,&$form_state)
{
	$proposal_id = arg(2);
    $form = array();
    $form['coding_guidelines'] = array(
    	'#type' => 'item',
    	'#markup' => t('<p><span style="color:#696969"><strong>All the guidelines should be followed without fail for timely acceptance of code. Failure to do so might result in rejection of your work. </strong></span></p>

<ol>
	<li dir="ltr">
	<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">Visit </span><a href="https://scilab.in/Completed_Books" target="_blank">this</a><span style="background-color:transparent; font-family:arial; font-size:11pt"> page and click on your proposed Scilab TBC. You will now get an option to download the Scilab code for the chosen TBC.&nbsp;</span></span></p>
	</li>
	<li dir="ltr">
	<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">System Requirement</span></span></p>

	<ol>
		<li dir="ltr">
		<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">You have to use Scilab 6.1.0 and above for creating the Xcos examples.&nbsp;</span></span></p>
		</li>
		<li dir="ltr">
		<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">You may use a different version of Scilab for executing the Scilab TBC code, if required.</span></span></p>
		</li>
		<li dir="ltr">
		<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">You may use a Windows/Linux/Mac OS</span></span></p>
		</li>
	</ol>
	</li>
	<li dir="ltr">
	<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">If the 3rd solved problem from chapter 1 is to be represented (Example 1.3), then your file must be named as Ex1_3.xcos.</span></span></p>
	</li>
	<li dir="ltr">
	<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">Dependency files</span></span></p>

	<ol>
		<li dir="ltr">
		<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">Use .sce extension for writing code while setting up the parameters for an Xcos diagram.</span></span></p>
		</li>
		<li dir="ltr">
		<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">Use .sci extension for writing code for user defined functions.</span></span></p>
		</li>
		<li dir="ltr">
		<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">These dependencies (if any) must be mentioned in a textbox clearly in the Xcos diagram.</span></span></p>
		</li>
	</ol>
	</li>
	<li dir="ltr">
	<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">Do not include problem statements/text from the book in the diagram.</span></span></p>
	</li>
	<li dir="ltr">
	<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">Plotting</span></span></p>

	<ol>
		<li dir="ltr">
		<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">All the plots must have appropriate labels on its axes.</span></span></p>
		</li>
		<li dir="ltr">
		<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">If a single plot contains several graphs then a legend must be added to help&nbsp;differentiate between different plots.</span></span></p>
		</li>
	</ol>
	</li>
	<li dir="ltr">
	<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">The symbols used in the code should be the same as those being used in the textbook. If you are unable to write a symbol, you can write the name of the symbol in words. For example, </span>"θ" <span style="background-color:transparent; font-family:arial; font-size:11pt">can be written as "theta".</span></span></p>
	</li>
	<li dir="ltr">
	<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">Check that you are using the correct formula while using in-built functions (For example, the angles must be converted to degree (or radians) before calculating the sine or cosine etc).</span></span></p>
	</li>
	<li dir="ltr">
	<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">Label the input/output blocks and superblocks (if applicable) used in Xcos diagram using a textbox.</span></span></p>
	</li>
	<li dir="ltr">
	<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">Label the numeric output blocks that display the results,&nbsp;with proper units.</span></span></p>
	</li>
	<li dir="ltr">
	<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">If the </span><strong>textbook contains errors</strong><span style="background-color:transparent; font-family:arial; font-size:11pt"> in calculations/results, </span><strong>mention </strong><span style="background-color:transparent; font-family:arial; font-size:11pt">the same in the block diagram itself by using the textbox.</span></span></p>

	<ol>
		<li dir="ltr">
		<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">If the degree of error is less than +/- 2 then </span><em>"The answers vary due to round off error"</em><span style="background-color:transparent; font-family:arial; font-size:11pt"> must be mentioned</span></span></p>
		</li>
		<li dir="ltr">
		<p dir="ltr"><span style="color:#696969"><span style="background-color:transparent; font-family:arial; font-size:11pt">If the degree of error is more than +/- 2 then </span><em>"The answer provided in the textbook is wrong"</em><span style="background-color:transparent; font-family:arial; font-size:11pt"> must be mentioned.</span></span></p>
		</li>
	</ol>
	</li>
</ol>
'),
    );
    $form['follow_guidelines'] = array(
            '#type' => 'checkbox',
            '#title' => t('I have read the coding guidelines'),
           // '#description' => 'Once you have submitted this option you are not able to upload more examples.',
            '#required' => TRUE,
        );
        $form['hidden_proposal_id'] = array(
        '#type' => 'hidden',
        '#value' => $proposal_id
    );
        $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit')
    );
        return $form;
}

function follow_guidelines_form_submit($form,&$form_state){
global $user;
if ($form_state['values']['follow_guidelines'] == 1) {
db_query('UPDATE textbook_companion_proposal SET follow_guidelines = 1 WHERE id = :proposal_id',array(':proposal_id' => $form_state['values']['hidden_proposal_id']));
}
drupal_goto('xcos-textbook-companion/code');
}