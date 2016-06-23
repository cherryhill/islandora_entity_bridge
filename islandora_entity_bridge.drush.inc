<?php

/**
 * @file
 * Drush commands for the Islandora Entity Bridge module.
 */

/**
 * Implements hook_drush_command().
 */
function islandora_entity_bridge_drush_command() {
  $items = array();

  $items['islandora_entity_bridge_populate_entity_bridge'] = array(
    'aliases' => array('ieb-populate'),
    'description' => 'Populate entity bridge',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'options' => array(
      'pid' => 'The PID to add to the entity bridge table',
    ),
  );

  return $items;
}

/**
 * Populate Entity Bridge Mappings.
 */
function drush_islandora_entity_bridge_populate_entity_bridge() {
  $one_pid = drush_get_option('pid', FALSE);

  $batch = array(
    'operations' => array(),
    'finished' => 'drush_islandora_entity_bridge_populate_entity_bridge_finished',
    'title' => t('Loading entities...'),
    'init_message' => t('Preparing to import image datastreams'),
    'progress_message' => t('Submitting content...'),
    'error_message' => t('Content could not get reimported'),
  );

  if ($one_pid) {
    $batch['operations'][] = array(
      'drush_islandora_entity_bridge_populate_entity_bridge_object', array($one_pid),
    );
  }
  else {
    $connection = islandora_get_tuque_connection();
    $query = 'SELECT ?object FROM <#ri> WHERE {
      ?object <dc:identifier> ?identifier ;
      } ORDER BY ?object';
    $results = $connection->repository->ri->sparqlQuery($query);
    foreach ($results as $result) {
      $pid = $result['object']['value'];
      $batch['operations'][] = array(
        'drush_islandora_entity_bridge_populate_entity_bridge_object', array($pid),
      );
    }
  }

  batch_set($batch);
  $batch =& batch_get();
  $batch['progressive'] = FALSE;
  drush_backend_batch_process();
}

/**
 * Add an entry to the Entity Bridge table
 *
 * @param string $pid
 *   PID of Fedora object to add
 * @param object $context
 *   Batch context
 */
function drush_islandora_entity_bridge_populate_entity_bridge_object($pid, &$context) {
  $context['message'] = t("Populating Fedora PID @pid", array('@pid' => $pid));
  $object = islandora_object_load($pid);
  islandora_entity_bridge_islandora_object_modified($object);
}

/**
 * End of batch processing
 */
function drush_islandora_entity_bridge_populate_entity_bridge_finished() {
  drupal_set_message('Successfully prepopulated entity bridge mapping table.');
}
