#include <drive.h>

#define FILE_URL "https://www.googleapis.com/drive/v3/files/"

/*
struct remote_file *drive_copy_file(struct drive *drive, struct remote_file *file, const char* name)
{
    struct string_builder str;
    struct json_builder json;
    struct net_result *net_result;
    struct remote_file *result;

    if (!drive->auth || !drive->auth->token)
        return 0;

    string_builder_init(&str, FILE_URL);
    string_builder_add_int(&str, file->id);
    string_builder_add_string(&str, "/copy");

    json_builder_init(&json);
    json_builder_add_object(&json);
    json_builder_add_key(&json, "name");
    json_builder_add_string(&json, name);
    json_builder_add_key(&json, "parents");
    json_builder_add_array(&json);
    for (int i = 0; i < file->parent_num; i++) {
        json_builder_add_string(&json, file->parents[i]);
    }
    json_builder_end_array(&json);
    json_builder_end_object(&json);

    net_result = net_post(str.data, json->buffer, drive->auth->token); 
    if (!net_result->success)
        return 0;

    remote_file_init(result, net_result->data);
    return result;
}

void drive_create_file(struct drive *drive, struct remote_file *parent)
{
    
}

void drive_send_file(struct drive *drive, struct local_file *file)
{
    if (!drive_fetch_changes(drive))
        return;
    
}

void drive_get_file(struct drive *drive, struct remote_file *file)
{
    net_get_file(drive->auth->net, file);
}

void drive_poll_changes(struct drive *drive, struct file_change * change)
{
    
}

static bool drive_fetch_changes(struct drive *drive)
{
    
    if (!drive->auth || !drive->auth->token)
        return false;
    

}
*/
