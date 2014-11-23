classdef wcSession
    properties
        urls        = {};
        maps        = {};
        map_data    = {};
        timestamps  = [];
        costs       = [];
        relevance   = [];
        rmse        = [];
        dVt         = [];
        npts        = 0;
    end
    
    
    methods
        
        function session = wcSession(cache_path)
            fclose all;
            full_path = [cache_path,'\session_info\logger.log'];
            fid       = fopen(full_path);
            
            if fid > 0
                while ~feof(fid)
                     try
                         session.urls{end+1}       = fscanf(fid,'%s',1);
                         session.maps{end+1}       = make_map_names(cache_path,session.urls{end});
                         session.timestamps(end+1) = fscanf(fid,'%f',1);
                         session.costs(end+1)      = fscanf(fid,'%f',1);
                         session.relevance(end+1)  = fscanf(fid,'%f',1);
                         session.rmse(end+1)       = fscanf(fid,'%f',1);
                         session.dVt(end+1)        = fscanf(fid,'%f',1);
                         session.npts              = session.npts + 1;
                     catch
                         break;
                     end
                end
                fclose(fid);
            else
                disp([cache_path,' is an invalid path.'])
            end
        end
        
        
        
        function session = loadPages(session)
            session.map_data = cell(session.npts-1,1);
            
            wb = waitbar(0,'Loading Map Data...');
            for idx = 1:session.npts
                waitbar(idx/(session.npts-1));
                session.map_data{idx} = wcPage(session.maps{idx});
            end
            delete(wb);
        end
        
        
        
        
        
        
    end
end


function url = make_map_names(path,url)
    c       = ((url<'a') | (url>'z')) & ((url<'A') | (url>'Z'));
    url(c)  = '_'; 
    url     = [path,'\state\',url,'.MAP'];
end