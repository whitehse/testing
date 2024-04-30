#!/usr/bin/bash

cat << EOF | sqlite3 $1
create table if not exists systems (
  id    integer primary key,
  name  text
);
create table if not exists events (
  rowid                  integer primary key autoincrement,
  system_id              integer,
  whence                 datetime,
  foreign key(system_id) references systems(id)
);
create index if not exists system_id_index ON events(system_id);
explain query plan
  select
    whence
  from
    events
  join
    systems
   on
    systems.id = events.system_id
  where
    id=1;
EOF
