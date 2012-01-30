<?php

/**
 * @file
 * Provides a controller for loading, creating, and saving works.
 */
class workController extends DrupalDefaultEntityController implements EntityAPIControllerInterface {
  
  /**
   * Stores our transaction object, necessary for pessimistic locking to work.
   */
  protected $controllerTransaction = NULL;

  protected function buildQuery($ids, $conditions = array(), $revision_id = FALSE) {
    return parent::buildQuery($ids, $conditions, $revision_id);
  }

  public function resetCache(array $ids = NULL) {
    parent::resetCache($ids);
    if (empty($this->entityCache) && !empty($this->controllerTransaction)) {
      // If we don't have any entity in our local cache anymore, we commit the
      // transaction so as to remove the locks we acquired.
      // This will not commit the translation directly. Drupal will commit
      // it as soon as possible given the state of the transaction stack.
      unset($this->controllerTransaction);
    }
  }

  /**
   * Deletes multiple works by ID.
   *
   * @param $ids
   *   An array of work IDs to delete.
   * @return
   *   TRUE on success, FALSE otherwise.
   */
  public function delete($ids, DatabaseTransaction $transaction = NULL) {
    
    $works = $ids ? $this->load($ids) : FALSE;
    if (!$works) {
      // Do nothing, incase invalid or no ids have been passed.
      return;
    }

    if (!isset($transaction)) {
      $transaction = db_transaction();
      $started_transaction = TRUE;
    }

    try {
      db_delete('work')
        ->condition('id', $ids, 'IN')
        ->execute();

      db_delete('work_revision')
        ->condition('id', $ids, 'IN')
        ->execute();
      
      // delete any related rows in the work_title table
      db_delete('work_title')
        ->condition('id', $ids, 'IN')
        ->execute();

      foreach ($works as $work_id => $work) {
        field_attach_delete('work', $work);
      }

      // Clear the page and block and work caches.
      $this->resetCache();

      // Ignore slave server temporarily.
      db_ignore_slave();

      // Invoke hook_ENTITY_view() to allow modules to add their additions.
      
      // Invoke the hook. If rules is there, use the rule funtion so that a rules
      // event is invoked too.
      if (module_exists('rules')) {
        rules_invoke_all($this->entityType . '_delete', $work);
      }
      else {
        module_invoke_all($this->entityType . '_delete', $work);
      }
      

      // Invoke the more generic hook_entity_view() to allow the same.
      module_invoke_all('entity_delete', $work, $this->entityType);

      return TRUE;
    }
    catch (Exception $e) {
      if (!empty($started_transaction)) {
        $transaction->rollback();
        watchdog_exception($this->entityType, $e);
      }
      throw $e;
    }
  }
  
  /**
   * (Internal use) Invokes a hook on behalf of the entity.
   *
   * For hooks that have a respective field API attacher like insert/update/..
   * the attacher is called too.
   */
  public function invoke($hook, $entity) {
    if (!empty($this->entityInfo['fieldable']) && function_exists($function = 'field_attach_' . $hook)) {
      $function($this->entityType, $entity);
    }

    // Invoke the hook. If rules is there, use the rule funtion so that a rules
    // event is invoked too.
    if (module_exists('rules')) {
      rules_invoke_all($this->entityType . '_' . $hook, $entity);
    }
    else {
      module_invoke_all($this->entityType . '_' . $hook, $entity);
    }
    // Invoke the respective entity level hook.
    if ($hook == 'presave' || $hook == 'insert' || $hook == 'update' || $hook == 'delete') {
      module_invoke_all('entity_' . $hook, $entity, $this->entityType);
    }
  }

  /**
   * Permanently saves the given entity.
   *
   * In case of failures, an exception is thrown.
   *
   * @param $entity
   *   The entity to save.
   * @param $transaction
   *   An optional transaction object to pass thru. If passed the caller is
   *   responsible for rolling back the transaction if something goes wrong.
   *
   * @return
   *   SAVED_NEW or SAVED_UPDATED depending on the operation performed.
   */
  public function save($work, DatabaseTransaction $transaction = NULL) {
    if (!isset($transaction)) {
      $transaction = db_transaction();
      $started_transaction = TRUE;
    }

    try {
      global $user;

      // Determine if we will be inserting a new work.
      $work->is_new = empty($work->id);

      // Set the timestamp fields.
      if (empty($work->created)) {
        $work->created = REQUEST_TIME;
      }
      $work->changed = REQUEST_TIME;

      $work->revision_timestamp = REQUEST_TIME;
      $update_work = TRUE;

      // Give modules the opportunity to prepare field data for saving.
      field_attach_presave('work', $work);

      // Set the enhanced flag on the work.
      $work->enhanced = $this->checkEnhanced($work);
      
      // When saving a new work revision, unset any existing $work->vid
      // to ensure a new revision will actually be created and store the old
      // revision ID in a separate property for work hook implementations.
      if (!$work->is_new && !empty($work->revision) && $work->vid) {
        $work->old_vid = $work->vid;
        unset($work->vid);
      }

      // If this is a new work...
      if ($work->is_new) {
        
        // Save the new work.
        $ret = drupal_write_record('work', $work);

        // Save the alternative titles in the joined table.
        foreach ($work->alternative_titles as $title) {
          $title->id = $work->id;
          drupal_write_record('work_title', $title);         
        }

        // Save the initial revision.
        $this->saveRevision($work, $user->uid);

        $op = 'insert';
      }
      else {
        // Save the updated work.
        $ret = drupal_write_record('work', $work, 'id');

        // Save the alternative titles.  Delete all the existing titles first
        db_delete('work_title')
          ->condition('id', $work->id)
          ->execute();
        // and save the new ones
        foreach ($work->alternative_titles as $title) {
          $title->id = $work->id;
          drupal_write_record('work_title', $title);         
        }

        // If a new work revision was requested, save a new record for that;
        // otherwise, update the work revision record that matches the value
        // of $work->vid.
        if (!empty($work->revision)) {
          $this->saveRevision($work, $user->uid);
        }
        else {
          $this->saveRevision($work, $user->uid, TRUE);
          $update_work = FALSE;
        }
        
        $op = 'update';
      }

      // If the revision ID is new or updated, save it to the work.
      if ($update_work) {
        db_update('work')
          ->fields(array('vid' => $work->vid))
          ->condition('id', $work->id)
          ->execute();
      }

      // Save fields.
      $function = 'field_attach_' . $op;
      $function('work', $work);

      module_invoke_all('entity_' . $op, $work, 'work');

      // Clear internal properties.
      unset($work->is_new);

      // Ignore slave server temporarily to give time for the saved order to be
      // propagated to the slave.
      db_ignore_slave();

      // return the value returned by drupal_write_record 
      return $ret;
    }
    catch (Exception $e) {
      $transaction->rollback();
      watchdog_exception('work', $e, NULL, WATCHDOG_ERROR);
      return FALSE;
    }
  }

  /**
   * Saves an work revision with the uid of the current user.
   *
   * @param $work
   *   The fully loaded work object.
   * @param $uid
   *   The user's uid for the current revision.
   * @param $update
   *   TRUE or FALSE indicating whether or not the existing revision should be
   *     updated instead of a new one created.
   */
  private function saveRevision($work, $uid, $update = FALSE) {
    // Hold on to the work's original creator_uid but swap in the revision's
    // creator_uid for the momentary write.
    $work->uid = 1; // hard coded for import
    $temp_uid = $work->uid;
    $work->uid = $uid;
    
    // Update the existing revision if specified.
    if ($update) {
      drupal_write_record('work_revision', $work, 'vid');
    }
    else {
      // Otherwise insert a new revision. This will automatically update $work
      // to include the vid.
      drupal_write_record('work_revision', $work);
    }

    // Reset the order's creator_uid to the original value.
    $work->uid = $temp_uid;
  }

  /**
   * Create a default work.
   *
   * @param $type
   *   The machine-readable type of the work.
   *
   * @return
   *   An work object with all default fields initialized.
   */
  public function create(array $values = array()) {
    return (object) array(
      'id' => '',
      'type' => $values['type'],
      'title' => '',
    );
  }

  /**
   * Implements EntityAPIControllerInterface.
   */
  public function export($entity, $prefix = '') {
    throw new Exception('Not implemented');
  }

  /**
   * Implements EntityAPIControllerInterface.
   */
  public function import($export) {
    throw new Exception('Not implemented');
  }

  /**
   * Builds a structured array representing the entity's content.
   *
   * The content built for the entity will vary depending on the $view_mode
   * parameter.
   *
   * @param $entity
   *   An entity object.
   * @param $view_mode
   *   View mode, e.g. 'full', 'teaser'...
   * @param $langcode
   *   (optional) A language code to use for rendering. Defaults to the global
   *   content language of the current request.
   * @return
   *   The renderable array.
   */
  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    // Remove previously built content, if exists.
    $entity->content = $content;
    $langcode = isset($langcode) ? $langcode : $GLOBALS['language_content']->language;

    // Add in fields.
    if (!empty($this->entityInfo['fieldable'])) {
      $entity->content += field_attach_view($this->entityType, $entity, $view_mode, $langcode);
    }

    // Invoke hook_ENTITY_view() to allow modules to add their additions.
    rules_invoke_all($this->entityType . '_view', $entity, $view_mode, $langcode);

    // Invoke the more generic hook_entity_view() to allow the same.
    module_invoke_all('entity_view', $entity, $this->entityType, $view_mode, $langcode);

    // Remove the build array information from the entity and return it.
    $build = $entity->content;
    unset($entity->content);

    return $build;
  }

  /**
   * Generate an array for rendering the given entities.
   *
   * @param $entities
   *   An array of entities to render.
   * @param $view_mode
   *   View mode, e.g. 'full', 'teaser'...
   * @param $langcode
   *   (optional) A language code to use for rendering. Defaults to the global
   *   content language of the current request.
   * @param $page
   *   (optional) If set will control if the entity is rendered: if TRUE
   *   the entity will be rendered without its title, so that it can be embeded
   *   in another context. If FALSE the entity will be displayed with its title
   *   in a mode suitable for lists.
   *   If unset, the page mode will be enabled if the current path is the URI
   *   of the entity, as returned by entity_uri().
   *   This parameter is only supported for entities which controller is a
   *   EntityAPIControllerInterface.
   * @return
   *   The renderable array.
   */
  public function view($entities, $view_mode = '', $langcode = NULL, $page = NULL) {
    // If no view mode is specified, use the first one available..
    if (!isset($this->entityInfo['view modes'][$view_mode])) {
      reset($this->entityInfo['view modes']);
      $view_mode = key($this->entityInfo['view modes']);
    }

    if (!empty($this->entityInfo['fieldable'])) {
      field_attach_prepare_view($this->entityType, $entities, $view_mode);
    }

    entity_prepare_view($this->entityType, $entities);
    $langcode = isset($langcode) ? $langcode : $GLOBALS['language_content']->language;
    $view = array();

    // Build the content array for each entity passed in.
    foreach ($entities as $key => $entity) {
      $build = entity_build_content($this->entityType, $entity, $view_mode, $langcode);

      // Add default properties to the array to ensure the content is passed
      // through the theme layer.
      $build += array(
        '#theme' => 'entity',
        '#entity_type' => $this->entityType,
        '#entity' => $entity,
        '#view_mode' => $view_mode,
        '#language' => $langcode,
        '#page' => $page,
      );

      // Allow modules to modify the structured entity.
      drupal_alter(array($this->entityType . '_view', 'entity_view'), $build, $this->entityType);
      $view[$this->entityType][$key] = $build;
    }

    return $view;
  }

  public function load($ids = array(), $conditions = array()) {
    
    // Use the inherited load method to load the field from the standard
    // schema table.
    $works = parent::load($ids, $conditions);
    foreach ($works as &$work) {
      
      // Alternative titles are stored in a seperate table.  Load them here
      // and add to the work object.
      $alternative_titles = db_select('work_title', 'ewt')
        ->fields('ewt')
        ->condition('id', $work->id, '=')
        ->execute()
        ->fetchAll();
      $work->alternative_titles = $alternative_titles;

    }
    return $works;
  }
  
  /**
   * A private function to check if a work has been enhanced in drupal.
   *
   * 'Enhanced' is currently defined as any work entity that has a value in any
   * of the introduction,synopsis or one_liner fields.
   */
  private function checkEnhanced($work) {
    if (! empty($work->introduction['und'])) {
      return 1;
    }
    if (! empty($work->synopsis['und'])) {
      return 1;
    }
    if (! empty($work->one_liner['und'])) {
      return 1;
    }
    return 0;
  }

}
