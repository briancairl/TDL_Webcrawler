classdef wcPage
    properties
        name        = '';
        url         = '';
        meta        = zeros(3,1);
        lout        = {};
        paths       = cell(3,1);
        
        
        graph_loc   = zeros(3,1);
    end
    
    
    methods
        
        function page = wcPage(mapname)
            fid = fopen(mapname);
            if fid > 0
                page.url        = fscanf(fid,'%s',1);
                page.name       = fscanf(fid,'%s',1);
                
                for idx = 1:3
                page.paths{idx} = fscanf(fid,'%s',1);
                end
                
                page.meta       = fscanf(fid,'%f',3);
                nlinks          = fscanf(fid,'%d',1);
                
                for idx = 1:nlinks
                page.lout{idx}  = fscanf(fid,'%s',1);
                end
                
                fclose(fid);
            else
                disp(['wcWarning :', mapname,' is an invalid path.'])
            end
        end
        
    end
end