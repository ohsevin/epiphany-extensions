/* header information */
#ifndef EPHY_DASHBOARD_EXTENSION_H
#define EPHY_DASHBOARD_EXTENSION_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define EPHY_TYPE_DASHBOARD_EXTENSION (ephy_dashboard_extension_get_type ())
#define EPHY_DASHBOARD_EXTENSION(o) (G_TYPE_CHECK_INSTANCE_CAST ((o, EPHY_TYPE_DASHBOARD_EXTENSION, EphyDashboardExtension))
#define EPHY_DASHBOARD_EXTENSION_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), EPHY_TYPE_DASHBOARD_EXTENSION, EphyDashboardExtensionClass))
#define EPHY_IS_DASHBOARD_EXTENSION(o) (G_TYPE_CHECK_INSTANCE_TYPE ((o), EPHY_TYPE_DASHBOARD_EXTENSION))
#define EPHY_IS_DASHBOARD_EXTENSION_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), EPHY_TYPE_DASHBOARD_EXTENSION))
#define EPHY_DASHBOARD_EXTENSION_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), EPHY_TYPE_DASHBOARD_EXTENSION, EphyDashboardExtensionClass))

typedef struct EphyDashboardExtension EphyDashboardExtension;
typedef struct EphyDashboardExtensionClass EphyDashboardExtensionClass;
typedef struct EphyDashboardExtensionPrivate EphyDashboardExtensionPrivate;

struct EphyDashboardExtensionClass
{
	GObjectClass parent_class;
};

struct EphyDashboardExtension
{
	GObject parent_instance;

	EphyDashboardExtensionPrivate *priv;
};

GType ephy_dashboard_extension_get_type (void);
GType ephy_dashboard_extension_register_type (GTypeModule *module);

G_END_DECLS

#endif

