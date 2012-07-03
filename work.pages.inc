<?php
// $Id$

/**
 * @file
 * Functions for displaying work pages.
 */

/**
 * Menu callback; Show list of work types we can add.
 */
function work_add_page() {
  $item = menu_get_item();
  $links = system_admin_menu_block($item);

  foreach ($links as $link) {
    $items[] = l($link['title'], $link['href'], $item['localized_options']) . ': ' . filter_xss_admin($link['description']);
  }

  return theme('item_list', array('items' => $items));
}

/**
 * Present an work submission form.
 */
function work_add($type) {
  global $user;

  $types = work_types();
  $type = isset($type) ? str_replace('-', '_', $type) : NULL;
  if (empty($types[$type])) {
    return MENU_NOT_FOUND;
  }

  $work = entity_get_controller('work')->create(array('type' => $type));

  drupal_set_title(t('Create @name', array('@name' => $types[$type]->name)), PASS_THROUGH);
  return drupal_get_form($type . '_work_form', $work);
}

/**
 * Menu callback; presents the work editing form, or redirects to delete confirmation.
 *
 * @param $work
 *   The work object to edit.
 */
function work_page_edit($work) {
  $types = work_types();
  drupal_set_title(t('<em>Edit @type</em> @title', array('@type' => $types[$work->type]->name, '@title' => $work->title)), PASS_THROUGH);

  return drupal_get_form($work->type . '_work_form', $work);
}

/**
 * Form builder; Displays the work add/edit form.
 *
 * @param $form
 * @param $form_state
 * @param $work
 *   The work object to edit, which may be brand new.
 */
function work_form($form, &$form_state, $work) {

  // Set the id and identify this as an work edit form.
  $form['#id'] = 'work-form';

  // Save the work for later, in case we need it.
  $form['#work'] = $work;
  $form_state['work'] = $work;

  // Common fields.
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Display title'),
    '#default_value' => $work->title,
    '#weight' => -5,
    '#required' => TRUE,
  );
  $form['identifying_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Identifying title'),
    '#default_value' => $work->identifying_title,
    '#weight' => -5,
    '#required' => TRUE,
    '#disabled' => TRUE,
  );
  $form['alpha_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Alphabetic title'),
    '#description' => t('The title used for sorting the works.'),
    '#default_value' => $work->alpha_title,
    '#weight' => -4,
    '#required' => FALSE,
    '#disabled' => TRUE,
  );
  if (isset($work->alternative_titles)) {
    if (! empty($work->alternative_titles)) {
      $form['alternative_titles'] = array(
        '#type' => 'fieldset',
        '#title' => t('Alternative titles'),
        '#weight' => -3,
      );

      foreach ($work->alternative_titles as $key => $alt_title) {
        $form['alternative_titles']['title' . $key] = array(
          '#type' => 'textfield',
          '#title' => t($alt_title->type),
          '#default_value' => $alt_title->title,
          '#disabled' => TRUE,
        );
      }
    }
  }
  $form['production_date'] = array(
    '#type' => 'textfield',
    '#title' => t('Production date'),
    '#description' => t('The production date for the work.'),
    '#default_value' => $work->production_date,
    '#weight' => -2,
    '#required' => FALSE,
    '#disabled' => TRUE,
  );
  $form['revision'] = array(
    '#access' => user_access('administer works'),
    '#type' => 'checkbox',
    '#title' => t('Create new revision'),
    '#default_value' => 0,
  );
  $form['promote'] = array(
    '#type' => 'checkbox',
    '#title' => t('Promote to homepage'),
    '#default_value' => $work->promote,
    '#required' => FALSE,
  );
  // Add the buttons.
  $form['buttons'] = array();
  $form['buttons']['#weight'] = 100;
  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 5,
    '#submit' => array('work_form_submit'),
  );
  if (!empty($work->id)) {
    $form['buttons']['delete'] = array(
      '#access' => user_access('delete works'),
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#weight' => 15,
      '#submit' => array('work_form_delete_submit'),
    );
  }

  $form['#validate'][] = 'work_form_validate';

  field_attach_form('work', $work, $form, $form_state);

  return $form;
}

function work_form_validate($form, &$form_state) {
  $work = $form_state['work'];

  // Field validation.
  field_attach_form_validate('work', $work, $form, $form_state);
}

function work_form_submit($form, &$form_state) {
  global $user;

  $work = &$form_state['work'];

  // Set the work's uid if it's being created at this time.
  if (empty($work->uid)) {
    $work->uid = $user->uid;
  }

  $work->title = $form_state['values']['title'];
  if (!empty($form_state['values']['identifying_title'])) {
    $work->identifying_title = $form_state['values']['identifying_title'];
  }
  if (!empty($form_state['values']['alpha_title'])) {
    $work->alpha_title = $form_state['values']['alpha_title'];
  }

  if (!empty($form_state['values']['production_date'])) {
    $work->production_date = $form_state['values']['production_date'];
  }

  $work->revision = $form_state['values']['revision'];

  $work->promote = $form_state['values']['promote'];

  // Notify field widgets.
  field_attach_submit('work', $work, $form, $form_state);

  // Save the work.
  work_save($work);

  // Notify the user.
  drupal_set_message(t('work saved.'));

  $form_state['redirect'] = 'work/' . $work->id;
}

function work_form_delete_submit($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  $work = $form['#work'];
  $form_state['redirect'] = array('work/' . $work->id . '/delete', array('query' => $destination));
}

/**
 * Displays a work.
 *
 * @param $work
 *   The work object to display.
 */
function work_page_view($work) {

  /**
   * Only allow displaying of the work if the status is set to 1.  Otherwise
   * return a 404. This doesn't allow content editors to view or edit a work
   * with status = 0, but since the published status is set in CID that
   * shouldn't be a problem.
   */
  if ($work->status) {
    $work_view = entity_view('work', array($work), 'full');
    return $work_view['work'][$work->id];
  }
  else {
    return drupal_not_found();
  }
}

/**
 * Form bulder; Asks for confirmation of work deletion.
 */
function work_delete_confirm($form, &$form_state, $work) {
  $form['#work'] = $work;
  // Always provide entity id in the same form key as in the entity edit form.
  $form['id'] = array('#type' => 'value', '#value' => $work->id);
  return confirm_form($form,
    t('Are you sure you want to delete %title?', array('%title' => $work->title)),
    'work/' . $work->id,
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Executes work deletion.
 */
function work_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    $work = work_load($form_state['values']['id']);
    work_delete($form_state['values']['id']);
    watchdog('work', '@type: deleted %title.', array('@type' => $work->type, '%title' => $work->title));

    $types = work_types();
    drupal_set_message(t('@type %title has been deleted.', array('@type' => $types[$work->type]->name, '%title' => $work->title)));
  }

  $form_state['redirect'] = '<front>';
}

/**
 * Menu callback; Displays a listing of recent works.
 *
 * This doesn't really work yet because our presentation code doesn't show
 * the title.
 */
function work_page_list_recent() {
  $content = array();

  $query = new EntityFieldQuery();
  $query
    ->entityCondition('entity_type', 'work')
    ->propertyOrderBy('created', 'DESC')
    ->range(0, 20);
  $result = $query->execute();

  $works = work_load_multiple(array_keys($result['work']));
  foreach ($works as $work) {
    $content[$work->id] = work_page_view($work, 'teaser');
  }

  return $content;
}

/**
 * Menu callback; Displays a list of enhanced works.
 */
function work_page_list() {

  global $base_url;

  $rows = array();

  $header = array(
    t('Title'),
    t('Type'),
    t('Edited'),
  );

  $query = db_select('work', 'w')
    ->fields('w', array('id', 'type', 'title', 'changed'))
    ->condition('enhanced', 1, '=')
    ->execute();

  foreach ($query as $work) {

    $cells = array();
    $cells[] = l($work->title, 'work/' . $work->id);
    $cells[] = $work->type;
    $cells[] = format_date($work->changed, 'short');
    $rows[] = array('data' => $cells);
  }

  $build['work_list_table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#attributes' => array('id' => ''),
    '#empty' => t('No works available.'),
  );
  return $build;
}
