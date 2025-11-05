"""
File: timezone_config.py
Project: Estufa Dashboard API
Created: Wednesday, 29th October 2025 9:19:32 am
Author: Klaus Begnis

Copyright (c) 2025 Estufa Dashboard. All rights reserved.
"""

import os
from zoneinfo import ZoneInfo

# Configurar timezone padrão para São Paulo
os.environ["TZ"] = "America/Sao_Paulo"

# Timezone de São Paulo (exportável para uso em outros módulos)
SAO_PAULO_TZ = ZoneInfo("America/Sao_Paulo")
